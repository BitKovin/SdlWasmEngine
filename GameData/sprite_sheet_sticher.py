from PIL import Image
import os
import array

dir = os.getcwd()

img_strips = []


img_dir = dir + '\\soda'
files = sorted(os.listdir(img_dir))
x_dim = 4096
y_dim = 4096
final_img = Image.new('RGBA', (x_dim,y_dim))
x_offset = 0
y_offset = 0
for file in files:
	print(file)
	print("\n")
	img = Image.open(img_dir + '\\' + file)
	final_img.paste(img, (x_offset, y_offset))
	x_offset += img.size[0]
	if x_offset+img.size[0] > x_dim:
		x_offset = 0
		y_offset += img.size[1]


final_img.save("spr_sheet.png")
print("SUCCESS!!!")

