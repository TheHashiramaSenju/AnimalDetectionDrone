from ultralytics import YOLO

model = YOLO('yolov8n.pt')
# Update the path to your YAML config file
data_yaml = '</Path/>'
# Train the model on your custom dataset; customize epochs, image size as needed.
model.train(data=data_yaml, epochs=50, imgsz=640)
