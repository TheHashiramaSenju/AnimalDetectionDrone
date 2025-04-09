import os

# Path to labels directory
labels_dir = "/home/path2/Miscfiles/Dataset/train/labels"  # Update for val/labels if needed

# Replace 'tranquil_area' with 'tranquil_zone'
def update_annotations(labels_dir):
    for filename in os.listdir(labels_dir):
        if filename.endswith(".txt"):
            file_path = os.path.join(labels_dir, filename)
            with open(file_path, "r") as file:
                lines = file.readlines()

            updated_lines = [line.replace("tranquil_area", "tranquil_zone") for line in lines]
            
            with open(file_path, "w") as file:
                file.writelines(updated_lines)

update_annotations(labels_dir)
print("Annotations updated successfully!")
