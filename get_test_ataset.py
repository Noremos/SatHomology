from PIL import Image
import numpy as np
import os

# Создаем выходную папку, если ее не существует
output_folder = 'test_dataset/crater'
os.makedirs(output_folder, exist_ok=True)

# Размеры изображения
width, height = 256, 256

# Количество изображений
num_images = 30

# Генерация изображений
for i in range(num_images):
    # Создаем массив с шумом
    noise = np.random.randint(0, 101, (height, width, 3), dtype=np.uint8)

    # Создаем изображение из массива
    img = Image.fromarray(noise, 'RGB')

    # Сохраняем изображение
    img.save(f'{output_folder}/image_{i+1}.png')

print('Генерация изображений завершена.')