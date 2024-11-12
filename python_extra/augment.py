import os
import cv2
import numpy as np
from collections import defaultdict

class ImageC:
    def __init__(self):
        self.name = ""
        self.augments = [""] * 10

class Results:
    def __init__(self, aug_name=""):
        self.aug_name = aug_name
        self.sum_of_cmp = 0.0
        self.sum_of_rsq = 0.0
        self.total = 0

    def add(self, result, rsq):
        self.sum_of_cmp += result
        self.sum_of_rsq += rsq
        self.total += 1

    def print_result(self):
        print(f"SSIM: {self.aug_name} {self.sum_of_cmp / self.total:.2f}")
        # print(f"RSQ:  {self.aug_name} {self.sum_of_rsq / self.total:.2f}")

def exe_augment():

    srcleanb = "/Users/sam/Edu/datasets/objects/eurosat_augment"
    source_files = defaultdict(lambda: ImageC())
    augment_number = {}

    for entry in os.listdir(srcleanb):
        path = os.path.join(srcleanb, entry)
        file_name = os.path.basename(path)

        if not (file_name.endswith(".png") or file_name.endswith(".jpg")):
            continue

        subtype_pos = file_name.find('-')
        ext_type = file_name.rfind('.')

        last_pos = ext_type if subtype_pos == -1 else subtype_pos
        only_name = file_name[:last_pos]
        img = source_files[only_name]

        if subtype_pos == -1:
            img.name = file_name
        else:
            sub_type = file_name[subtype_pos + 1:ext_type]
            if sub_type not in augment_number:
                augment_number[sub_type] = len(augment_number)

            sub_type_pos = augment_number[sub_type]
            img.augments[sub_type_pos] = file_name

    num_to_name_map = [Results(name) for name in augment_number]
    aug_size = len(num_to_name_map)

    from skimage.metrics import structural_similarity as ssim
    from skimage.metrics import mean_squared_error

    for img in source_files.values():
        src = cv2.imread(os.path.join(srcleanb, img.name), cv2.IMREAD_GRAYSCALE)


        for i in range(aug_size):
            assert img.augments[i] != ""

            aug = cv2.imread(os.path.join(srcleanb, img.augments[i]), cv2.IMREAD_GRAYSCALE)

            mse_noise = mean_squared_error(src, aug)
            ssim_noise = ssim(src, aug, data_range=aug.max() - aug.min())

            num_to_name_map[i].add(ssim_noise, mse_noise)


    for res in num_to_name_map:
        res.print_result()

    return []

exe_augment()