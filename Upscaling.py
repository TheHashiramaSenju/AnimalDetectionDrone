from PIL import Image
import os

# Paths to your dataset folders
dataset_folders = [
    "/All dataset folders path/"
]

# Resized image output directory
output_base = "/home/newton/Miscfiles/Dataset/resized/"
new_size = (640, 640)  # Desired size for YOLO training

for folder in dataset_folders:
    # Create a corresponding output folder
    output_folder = folder.replace("train/images.cv_7leof0z6bpdlqfwktit0n8/", "resized/")
    os.makedirs(output_folder, exist_ok=True)

    # Resize all images
    for filename in os.listdir(folder):
        if filename.endswith(".jpg") or filename.endswith(".png"):
            img_path = os.path.join(folder, filename)
            img = Image.open(img_path)
            img_resized = img.resize(new_size)
            img_resized.save(os.path.join(output_folder, filename))

print("Resizing complete! Check the 'resized/' folder for your processed images.")
