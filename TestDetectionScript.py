import cv2
import numpy as np
from ultralytics import YOLO

MODEL_PATH = '/YOLO weights/'  # Path to your YOLO model weights
CONF_THRESHOLD = 0.5   
NMS_THRESHOLD = 0.4  
REASSIGN_TRACKER_INTERVAL = 10  
CLASSES = ['animal', 'tranquil_zone']   




class MultiObjectTracker:
    def __init__(self):
        self.trackers = {}   
        self.next_object_id = 0   
        self.tracker_lifetimes = {}  

    def add_tracker(self, frame, bbox):
        tracker = cv2.legacy.TrackerCSRT_create()   
        tracker.init(frame, bbox)
        self.trackers[self.next_object_id] = tracker
        self.tracker_lifetimes[self.next_object_id] = 0
        self.next_object_id += 1


    def update_trackers(self, frame):
        updated_trackers = {}
        updated_lifetimes = {}

        for obj_id, tracker in self.trackers.items():
            success, bbox = tracker.update(frame)
            if success:
                updated_trackers[obj_id] = tracker
                updated_lifetimes[obj_id] = 0  
            else:
                updated_lifetimes[obj_id] = self.tracker_lifetimes.get(obj_id, 0) + 1

        self.trackers = updated_trackers
        self.tracker_lifetimes = updated_lifetimes

    def get_tracked_objects(self, frame, max_lifetime=REASSIGN_TRACKER_INTERVAL):
        tracked_objects = []
        for obj_id, tracker in self.trackers.items():
            success, bbox = tracker.update(frame)
            if success:
                x, y, w, h = map(int, bbox)
                tracked_objects.append((obj_id, (x, y, x + w, y + h)))

            if self.tracker_lifetimes.get(obj_id, 0) > max_lifetime:
                del self.trackers[obj_id]
                del self.tracker_lifetimes[obj_id]

        return tracked_objects

def nms_boxes(boxes, scores, iou_threshold):

    indices = cv2.dnn.NMSBoxes(boxes.tolist(), scores.tolist(), CONF_THRESHOLD, iou_threshold)
    return indices.flatten() if len(indices) > 0 else []




def main():
    try:
        model = YOLO(MODEL_PATH)  
    except FileNotFoundError:
        print(f"Error: YOLO weights file not found at {MODEL_PATH}. Please update the path.")
        return

    tracker_manager = MultiObjectTracker()

    
    cap = cv2.VideoCapture(0)  
    if not cap.isOpened():
        print("Error: Cannot open camera.")
        return

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Failed to read frame.")
            break

        results = model(frame)
        detections = results[0].boxes.xyxy.cpu().numpy()
        scores = results[0].boxes.conf.cpu().numpy()
        classes = results[0].boxes.cls.cpu().numpy()

        boxes = [list(map(int, det)) for det in detections]
        indices = nms_boxes(np.array(boxes), scores, NMS_THRESHOLD)
        filtered_detections = [(boxes[i], int(classes[i])) for i in indices]

        tracker_manager.update_trackers(frame)

        for bbox, cls_id in filtered_detections:
            x1, y1, x2, y2 = bbox
            w, h = x2 - x1, y2 - y1
            tracker_manager.add_tracker(frame, (x1, y1, w, h))


        tracked_objects = tracker_manager.get_tracked_objects(frame)

        for obj_id, (x1, y1, x2, y2) in tracked_objects:
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(frame, f"ID: {obj_id}", (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

        for bbox, cls_id in filtered_detections:
            x1, y1, x2, y2 = bbox
            label = CLASSES[cls_id] if cls_id < len(CLASSES) else "Unknown"
            color = (255, 0, 0) if label == "tranquil_zone" else (0, 255, 255)
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            cv2.putText(frame, label, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

        cv2.imshow("YOLO + OpenCV Tracking", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
