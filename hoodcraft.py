#!/usr/bin/env python2
# Hoodcraft
# Copyright 2017 Declan Hoare
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as published by
# the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
# Hood editor for POSTAL

import PIL.Image, pickle, atexit, shutil, struct, tempfile, os, RSPiX
import cStringIO, distutils.spawn, subprocess, PIL.ImageDraw, Tkinter
import tkMessageBox, tkFileDialog, sys, PIL.ImageTk

class invalid_data(Exception):
	pass

class rspix_error(Exception):
	pass

shortstruct = struct.Struct("<h")
ushortstruct = struct.Struct("<H")
ulongstruct = struct.Struct("<L")
def read_u16(fobj):
	return ushortstruct.unpack(fobj.read(2))[0]
def read_u32(fobj):
	return ulongstruct.unpack(fobj.read(4))[0]
def write_u16(fobj, val):
	fobj.write(ushortstruct.pack(val))
def write_u32(fobj, val):
	fobj.write(ulongstruct.pack(val))

colourstruct = struct.Struct("<BBBB")
def pack_rgba(rgba):
	return ulongstruct.unpack(colourstruct.pack(*rgba))[0]
def unpack_rgba(u32):
	return colourstruct.unpack(ulongstruct.pack(u32))

def layer_num_to_bit(layer):
	return 0 if layer == 15 else (1 << layer)

def interlaced(a, b):
	for i in xrange(min(len(a), len(b))):
		yield a[i]
		yield b[i]

sak_magic = 0x204b4153
sak_version = 1

ralpha_magic = "RALPHA\0"
ralpha_version = 1

# pickle version 2 fails to deserialise PIL.Image
pickle_version = 1

postal_ppal = [[0, 0, 0, 0], [1, 191, 0, 0], [2, 0, 191, 0], [3, 191, 191, 0], [4, 0, 0, 191], [5, 191, 0, 191], [6, 0, 191, 191], [7, 192, 192, 192], [8, 192, 220, 192], [9, 166, 202, 240], [106, 16, 39, 32], [107, 28, 54, 46], [108, 48, 72, 65], [109, 68, 90, 83], [110, 93, 111, 106], [111, 134, 148, 144], [112, 175, 184, 181], [113, 218, 223, 221], [114, 13, 16, 32], [115, 22, 28, 55], [116, 30, 37, 67], [117, 48, 54, 82], [118, 75, 80, 105], [119, 100, 105, 126], [120, 130, 134, 151], [121, 160, 163, 176], [122, 9, 1, 1], [123, 24, 2, 2], [124, 47, 5, 5], [125, 71, 15, 15], [126, 97, 48, 48], [127, 120, 79, 79], [128, 146, 112, 112], [129, 174, 150, 150], [130, 11, 5, 2], [131, 28, 15, 6], [132, 55, 29, 13], [133, 82, 47, 25], [134, 106, 76, 57], [135, 128, 103, 86], [136, 152, 131, 118], [137, 179, 164, 154], [138, 25, 18, 11], [139, 65, 47, 30], [140, 129, 94, 59], [141, 180, 133, 87], [142, 190, 150, 110], [143, 200, 166, 132], [144, 210, 182, 155], [145, 222, 202, 181], [146, 17, 36, 41], [147, 24, 51, 58], [148, 32, 64, 71], [149, 53, 82, 89], [150, 75, 100, 106], [151, 105, 126, 131], [152, 135, 152, 156], [153, 172, 184, 187], [154, 9, 7, 11], [155, 24, 18, 30], [156, 47, 35, 59], [157, 71, 55, 87], [158, 97, 82, 110], [159, 120, 108, 132], [160, 146, 136, 155], [161, 174, 167, 181], [162, 17, 15, 5], [163, 45, 39, 15], [164, 89, 77, 29], [165, 127, 111, 47], [166, 144, 131, 76], [167, 161, 149, 103], [168, 178, 169, 131], [169, 199, 192, 164], [170, 29, 28, 17], [171, 74, 71, 44], [172, 146, 141, 87], [173, 203, 196, 125], [174, 210, 204, 143], [175, 217, 212, 159], [176, 224, 220, 177], [177, 232, 229, 198], [178, 40, 37, 37], [179, 55, 51, 51], [180, 73, 69, 69], [181, 116, 108, 108], [182, 140, 131, 131], [183, 174, 162, 162], [184, 217, 202, 202], [185, 237, 228, 228], [186, 3, 2, 4], [187, 23, 15, 28], [188, 39, 31, 44], [189, 53, 46, 58], [190, 70, 63, 74], [191, 81, 75, 84], [192, 106, 101, 109], [193, 136, 132, 139], [194, 19, 6, 6], [195, 49, 16, 17], [196, 96, 31, 34], [197, 136, 50, 53], [198, 153, 78, 81], [199, 168, 105, 107], [200, 184, 133, 135], [201, 203, 165, 166], [202, 255, 228, 173], [203, 255, 231, 151], [204, 255, 231, 136], [205, 255, 231, 123], [206, 255, 222, 151], [207, 255, 222, 140], [208, 255, 222, 132], [209, 255, 222, 123], [210, 255, 223, 115], [211, 255, 222, 107], [212, 255, 222, 99], [213, 255, 214, 135], [214, 255, 222, 88], [215, 255, 214, 111], [216, 255, 214, 99], [217, 255, 214, 90], [218, 255, 214, 82], [219, 255, 214, 71], [220, 255, 206, 84], [221, 255, 206, 69], [222, 252, 200, 103], [223, 255, 205, 57], [224, 255, 203, 49], [225, 255, 200, 41], [226, 255, 198, 30], [227, 255, 188, 28], [228, 255, 189, 15], [229, 255, 177, 11], [230, 255, 174, 0], [231, 254, 161, 0], [232, 251, 142, 0], [233, 235, 130, 0], [234, 224, 111, 0], [235, 175, 113, 22], [236, 211, 92, 0], [237, 189, 73, 0], [238, 160, 74, 0], [239, 158, 45, 0], [240, 146, 27, 0], [241, 128, 16, 0], [242, 100, 23, 0], [243, 62, 8, 0], [244, 25, 2, 0], [245, 0, 0, 0], [246, 255, 251, 240], [247, 160, 160, 164], [248, 128, 128, 128], [249, 255, 0, 0], [250, 0, 255, 0], [251, 255, 255, 0], [252, 0, 0, 255], [253, 255, 0, 255], [254, 0, 255, 255], [255, 255, 255, 255]]

class sak_reader:
	def __init__(self, fobj):
		if read_u32(fobj) != sak_magic:
			raise invalid_data("This is not a SAK file.")
		if read_u32(fobj) != sak_version:
			raise invalid_data("This is not a SAK v1 file.")
		self.num_pairs = read_u16(fobj)
		self.fobj = fobj
	def read_file(self, fname):
		self.fobj.seek(10)
		for i in xrange(self.num_pairs):
			test = ""
			while True:
				c = self.fobj.read(1)
				if c == "\0" or c == "":
					break
				else:
					test += c
			if test == fname:
				offset = read_u32(self.fobj)
				if i == self.num_pairs - 1:
					self.fobj.seek(offset)
					return self.fobj.read()
				else:
					while True:
						c = self.fobj.read(1)
						if c == "\0" or c == "":
							break
					length = read_u32(self.fobj) - offset
					self.fobj.seek(offset)
					return self.fobj.read(length)
		raise IOError(fname + " does not exist in this SAK.")

def write_sak(fobj, files):
	write_u32(fobj, sak_magic)
	write_u32(fobj, sak_version)
	write_u16(fobj, len(files))
	pos = 10 + sum(len(fname) + 5 for fname in files.iterkeys())
	for fname, data in files.iteritems():
		fobj.write(fname + "\0")
		write_u32(fobj, pos)
		pos += len(data)
	for data in files.itervalues():
		fobj.write(data)

def separate(l, n):
	for i in range(0, len(l), n):
		yield l[i:i + n]

def flatten(l):
	try:
		return sum(l, l[0].__class__())
	except IndexError:
		return l.__class__()

try:
	homer = sak_reader(open("res/hoods/homer.sak", "rb"))
except IOError:
	Tkinter.Tk().withdraw()
	tkMessageBox.showerror("Hoodcraft", "Could not find homer.sak. Are you sure Hoodcraft is in the POSTAL directory?")
	sys.exit(1)
except invalid_data:
	Tkinter.Tk().withdraw()
	tkMessageBox.showerror("Hoodcraft", "homer.sak is corrupt.")
	sys.exit(1)
atexit.register(homer.fobj.close)

standard_images = {fname: PIL.Image.open(cStringIO.StringIO(homer.read_file("hoods/homer/homer" + fname + ".bmp"))) for fname in [".emptybar", ".emptybarselected", ".fullbar", ".fullbarselected", ".topbar"]}
homer_palette = PIL.Image.open(cStringIO.StringIO(homer.read_file("hoods/homer/homer.bmp"))).getpalette()
homer_alphas = {fname: homer.read_file("hoods/homer/homer." + fname + ".alpha") for fname in ["ambient", "spot"]}

gimp_path = distutils.spawn.find_executable("gimp")
if not gimp_path:
	Tkinter.Tk().withdraw()
	tkMessageBox.showerror("Hoodcraft", "Could not find GIMP. Make sure it's installed and in the PATH.")
	sys.exit(1)

class terrain_region:
	def __init__(self):
		self.height = 0
		self.walkable = True
		self.lit = False
	def to_s16(self):
		return self.height | ((not self.walkable) << 8) | (self.lit << 9)

class vector_map:
	def __init__(self, width, height):
		self.width = width
		self.height = height
		self.regions = []
		self.pim = PIL.Image.new("RGBA", (width, height))
	
	def redraw_internal(self):
		self.pim = PIL.Image.new("RGBA", (self.width, self.height))
		draw = PIL.ImageDraw.Draw(self.pim)
		for i, region in enumerate(self.regions):
			draw.polygon(region[0], unpack_rgba(i + 1))
	
	def find_region(self, value):
		return next(i for i, (l, v) in enumerate(self.regions) if v is value)
	
	def add_region(self, lines, value):
		self.regions.append((lines, value))
		self.redraw_internal()
	
	def remove_region(self, value):
		self.regions.pop(self.find_region(value))
		self.redraw_internal()
	
	# offset = -1 -> move down
	# offset = +1 -> move up
	def swap_region(self, value, offset):
		index = self.find_region(value)
		if index + offset < 0 or index + offset >= len(self.regions):
			return
		(self.regions[index], self.regions[index + offset]) = (self.regions[index + offset], self.regions[index])
	
	def get_at(self, x, y):
		packed = pack_rgba(self.pim.getpixel((x, y)))
		return self.regions[packed - 1][1] if packed else None
	
	def visualise_to(self, draw, selected = None):
		for lines, value in self.regions:
			colour = (255, 255, 0, 255) if value is selected else (0, 255, 0, 64)
			draw.polygon(lines, colour)
	
	def to_multigrid(self):
		ret = RSPiX.RMultiGrid()
		ret.Alloc(self.width, self.height)
		for y in xrange(self.height):
			for x in xrange(self.width):
				item = self.get_at(x, y)
				value = item.to_s16() if item else 0
				ret.SetValueUncompressed(value, x, y)
		ret.Compress(64, 64)
		return ret

class hood:
	def __init__(self):
		# hood data
		self.background = None
		self.alpha = [None] * 8
		self.opaque = [None] * 8
		self.terrain_map = None
		self.name = ""
		
		# display data for editor
		self.background_name = "(none)"
		self.alpha_names = ["(none)"] * 8
		self.opaque_names = ["(none)"] * 8
	def build(self):
		if not self.background or not self.terrain_map or not self.name or self.name.isspace():
			raise invalid_data("Your project must have a background image, terrain map, and name.")
		tmpdir = tempfile.mkdtemp()
		try:
			# quantiser
			my_images = dict(standard_images)
			my_images[""] = self.background.convert("RGBA")
			for i, im in enumerate(self.alpha):
				if im:
					my_images["0" + str(i) + "a"] = im
			for i, im in enumerate(self.opaque):
				if im:
					my_images["0" + str(i) + "o"] = im
			for k, v in my_images.iteritems():
				v.save(tmpdir + "/" + k + ".png", "png")
			pimages = my_images.keys()
			(pimages[0], pimages[pimages.index("")]) = ("", pimages[0]) # base must be first
			pimages = [tmpdir + "/" + k + ".png" for k in pimages]
			proc = subprocess.Popen([gimp_path, "-i", "--batch-interpreter=python-fu-eval", "-b", """
pimages = {0}

image = pdb.gimp_file_load(pimages[0], pimages[0])
for fname in pimages[1:]:
	image.add_layer(pdb.gimp_file_load_layer(image, fname))

pdb.gimp_image_convert_indexed(image, 0, 0, 96, False, False, "")

for fname, layer in zip(reversed(pimages), image.layers):
	image_copy = image.duplicate()
	image_copy.resize(layer.width, layer.height)
	pdb.gimp_file_save(image_copy, layer, fname, fname)

pdb.gimp_quit(True)
	""".format(pimages)])
			proc.wait()
			if proc.returncode != 0:
				raise subprocess.CalledProcessError("GIMP returned failure code " + str(proc.returncode))
			for k in my_images.keys():
				my_images[k] = PIL.Image.open(tmpdir + "/" + k + ".png")
			
			# ppalmap
			free_start = 0
			while free_start == postal_ppal[free_start][0]:
				free_start += 1
			entries = [e[1:] for e in postal_ppal]
			free_zone = 256 - len(entries)
			for im in my_images.itervalues():
				imagepal = list(separate(im.getpalette(), 3))[:free_zone]
				newpal = flatten(entries[:free_start] + imagepal + entries[free_start:])
				t = im.info.get("transparency", None)
				newdata = [0 if p == t else p + free_start for p in im.getdata()]
				im.putpalette(newpal)
				im.putdata(newdata)
			
			files = {}
			
			# png2spry
			for k, im in my_images.iteritems():
				if k.startswith("0"): # spry
					cur = list(separate(im.getpalette(), 3))
					target = list(separate(my_images[""].getpalette(), 3))
					cur[0] = target[0] # transparent
					palmap = [target.index(c) for c in cur]
					im.putpalette(flatten(target))
					im.putdata([palmap[c] for c in im.getdata()])
					sprl = []
					keepalive = [] # work around SWIG segfaults
					gw = (im.width + 63) / 64
					gh = (im.height + 63) / 64
					for gy in xrange(gh):
						for gx in xrange(gw):
							left = (im.width / 2) + (gx - (gw / 2)) * 64
							right = left + 64
							left = left if left > 0 else 0
							right = right if right < im.width else im.width
							top = (im.height / 2) + (gy - (gh / 2)) * 64
							bottom = top + 64
							top = top if top > 0 else 0
							bottom = bottom if bottom < im.height else im.height
							chunk = im.crop((left, top, right, bottom))
							data = list(chunk.getdata())
							if not any(data): # empty chunk
								continue
							ctop = 0
							cbottom = chunk.height
							cleft = 0
							cright = chunk.width
							for row in separate(data, chunk.width):
								if any(row):
									break
								else:
									ctop += 1
							for row in reversed(list(separate(data, chunk.width))):
								if any(row):
									break
								else:
									cbottom -= 1
							for x in xrange(chunk.width):
								if any(data[p] for p in xrange(x, len(data), chunk.height)):
									break
								else:
									cleft += 1
							for x in xrange(chunk.width - 1, -1, -1):
								if any(data[p] for p in xrange(x, len(data), chunk.height)):
									break
								else:
									cright -= 1
							chunk = chunk.crop((cleft, ctop, cright, cbottom))
							top += ctop
							left += cleft
							rim = RSPiX.RImage()
							rim.Init()
							outbmp = cStringIO.StringIO()
							chunk.save(outbmp, "bmp")
							mydata = outbmp.getvalue()
							outbmp.close()
							mem = RSPiX.allocateFile(len(mydata))
							rfile = RSPiX.RFile()
							rfile.Open(mem, len(mydata), rfile.LittleEndian)
							rfile.Write(mydata, len(mydata))
							rfile.Seek(0, os.SEEK_SET)
							ec = rim.Load(rfile)
							if ec != 0:
								raise rspix_error(ec)
							ec = rim.Convert(rim.FSPR8)
							if ec != rim.FSPR8:
								raise rspix_error(ec)
							rim.DestroyPalette()
							sprl.append(RSPiX.RSprite(left, top, len(sprl), 0, chunk.width, chunk.height, 0, rim)) 
							keepalive.append(rim)
					spry = RSPiX.RSpry()
					for sprite in sprl:
						spry.m_listSprites.InsertTail(sprite)
					rfile = RSPiX.RFile()
					rfile.Open(RSPiX.fopen(tmpdir + "/" + k + ".say", "wb"), rfile.LittleEndian, rfile.Binary)
					spry.Save(rfile)
					rfile.Close()
					# RSPiX and SWIG compete over ownership of the
					# sprites contained in the Spry, causing a
					# segmentation fault when the Spry is deallocated.
					# The solution is to manually detach the sprites
					# from the Spry.
					while spry.m_listSprites.GetHead():
						spry.m_listSprites.RemoveHead()
					with open(tmpdir + "/" + k + ".say", "rb") as fobj:
						files["hoods/" + self.name + "/" + self.name + k + ".say"] = fobj.read()
				else: # bmp
					outbmp = cStringIO.StringIO()
					im.save(outbmp, "bmp")
					files["hoods/" + self.name + "/" + self.name + k + ".bmp"] = outbmp.getvalue()
					outbmp.close()
			
			# alphagen
			our_palette = my_images[""].getpalette()
			for k, alpha in homer_alphas.iteritems():
				newalpha = str(ralpha_magic)
				newalpha += shortstruct.pack(ralpha_version)
				depth = shortstruct.unpack(alpha[9:11])[0]
				newalpha += shortstruct.pack(depth)
				for i in xrange(256 * depth):
					orig_start = ord(alpha[11 + i]) * 3
					(orig_r, orig_g, orig_b) = homer_palette[orig_start:orig_start + 3]
					mn = 768
					for j in xrange(256):
						new_start = j * 3
						(new_r, new_g, new_b) = our_palette[new_start:new_start + 3]
						diff = abs(new_r - orig_r) + abs(new_g - orig_g) + abs(new_b - orig_b)
						if diff < mn:
							mn = diff
							new_entry = j
					newalpha += chr(new_entry)
				files["hoods/" + self.name + "/" + self.name + "." + k + ".alpha"] = newalpha
			
			# malphagen
			with open(tmpdir + "/base.bmp", "wb") as fobj:
				fobj.write(files["hoods/" + self.name + "/" + self.name + ".bmp"])
			RSPiX.malphagen(tmpdir + "/base.bmp", tmpdir + "/transparency.multialpha")
			with open(tmpdir + "/transparency.multialpha", "rb") as fobj:
				files["hoods/" + self.name + "/" + self.name + ".transparency.multialpha"] = fobj.read()
			
			# compile terrain map
			rfl = RSPiX.RFile()
			rfl.Open(RSPiX.fopen(tmpdir + "/mp1", "wb"), rfl.LittleEndian, rfl.Binary)
			self.terrain_map.to_multigrid().Save(rfl)
			rfl.Close() # also closes fopen
			with open(tmpdir + "/mp1", "rb") as fobj:
				files["hoods/" + self.name + "/" + self.name + ".mp1"] = fobj.read()
			
			# generate layer map
			rmg = RSPiX.RMultiGrid()
			rmg.Alloc(self.background.width, self.background.height)
			for y in xrange(self.background.height):
				for x in xrange(self.background.width):
					value = 0
					for k, v in my_images.iteritems():
						if k.startswith("0"):
							l = int(k[1]) * 2 # opaque/alpha namespace interlaced
							if k[2] == "a": # if alpha, odd
								l += 1
							value |= layer_num_to_bit(l) if v.getpixel((x, y)) else 0
					rmg.SetValueUncompressed(value, x, y)
			rmg.Compress(64, 64)
			rfl = RSPiX.RFile()
			rfl.Open(RSPiX.fopen(tmpdir + "/mp2", "wb"), rfl.LittleEndian, rfl.Binary)
			rmg.Save(rfl)
			rfl.Close()
			with open(tmpdir + "/mp2", "rb") as fobj:
				files["hoods/" + self.name + "/" + self.name + ".mp2"] = fobj.read()
			
			# copy x-ray mask verbatim
			files["hoods/" + self.name + "/" + self.name + ".xraymask.bmp"] = homer.read_file("hoods/homer/homer.xraymask.bmp")
			
			# saktool
			with open("res/hoods/" + self.name + ".sak", "wb") as fobj:
				write_sak(fobj, files)
		finally:
			# clean up
			shutil.rmtree(tmpdir)

project = hood()
project_path = None
modified = False

file_dialog_options = {"defaultextension": ".hood",
	"filetypes": [("Hoodcraft project", "*.hood"), ("All files", "*")]}

def localise_path(path):
	if not path:
		return None
	return path.replace("\\", os.sep).replace("/", os.sep)

def normalise_path(path):
	return path.replace(os.sep, "/")

def update_title():
	root.title(("*" if modified else "") + (project_path if project_path else "Untitled") + " - Hoodcraft")

def split_path():
	if project_path:
		return project_path.rsplit(os.sep, 1)
	else:
		return (None, "Untitled")

def new():
	global project, project_path, modified
	if editor:
		hide_terrain_editor()
	if verify_close():
		project = hood()
		project_path = None
		modified = False
		update_bgimage_label()
		for i in range(8):
			update_alpha_label(i)
			update_opaque_label(i)
		update_title()

def choose_name():
	global project_path
	project_dir, project_file = split_path()
	new_project_path = localise_path(tkFileDialog.asksaveasfilename(
		initialdir = project_dir,
		initialfile = project_file,
		parent = root,
		**file_dialog_options))
	if not new_project_path:
		return False
	project_path = new_project_path
	return True

def save():
	global modified
	if not project_path:
		if not choose_name():
			return False
	try:
		with open(normalise_path(project_path), "wb") as fobj:
			pickle.dump(project, fobj, pickle_version)
	except Exception as ex:
		tkMessageBox.showerror("Hoodcraft", "Failed to save {0}: {1}".format(project_path, ex))
		return False
	modified = False
	update_title()
	return True

def save_as():
	if not choose_name():
		return False
	return save()

def verify_close():
	if modified:
		response = tkMessageBox.askyesnocancel(
			title = "Hoodcraft",
			message = "You have unsaved changes. Would you like to save {0} before closing it?".format(project_path if project_path else "Untitled"),
			parent = root)
		if response:
			return save()
		else:
			return response is not None
	return True

def verify_quit():
	if verify_close():
		root.destroy()
		sys.exit()

def open_file():
	global project, project_path, modified
	if editor:
		hide_terrain_editor()
	if verify_close():
		project_dir, project_file = split_path()
		new_project_path = localise_path(tkFileDialog.askopenfilename(
			initialdir = project_dir,
			initialfile = project_file,
			parent = root,
			**file_dialog_options))
		if not new_project_path:
			return
		try:
			with open(normalise_path(new_project_path), "rb") as fobj:
				project = pickle.load(fobj)
		except Exception as ex:
			tkMessageBox.showerror("Hoodcraft", "Failed to open {0}: {1}".format(project_path, ex))
			return
		tk_hood_name.set(project.name)
		project_path = new_project_path
		update_bgimage_label()
		for i in xrange(8):
			update_alpha_label(i)
			update_opaque_label(i)
		modified = False
		update_title()

def build():
	try:
		project.build()
		tkMessageBox.showinfo("Hoodcraft", "{0} built successfully.".format(project.name))
	except Exception as ex:
		tkMessageBox.showerror("Hoodcraft", "Failed to build {0}: {1}".format(project.name, ex))

def update_project_name(name, index, mode):
	global modified
	project.name = tk_hood_name.get()
	modified = True
	update_title()

image_filetypes = [("PNG images", "*.png"),
			("BMP images", "*.bmp"),
			("JPEG images", "*.jpg"),
			("JPEG images", "*.jpeg"),
			("GIF images", "*.gif"),
			("All files", "*")]

def browse_bgimage():
	global modified
	if editor:
		hide_terrain_editor()
	path = localise_path(tkFileDialog.askopenfilename(
		parent = root,
		filetypes = image_filetypes))
	if not path:
		return
	try:
		new_background = PIL.Image.open(normalise_path(path))
	except Exception as ex:
		tkMessageBox.showerror("Hoodcraft", "Failed to open {0}: {1}".format(path, ex))
		return
	if project.background:
		if project.background.width != new_background.width or project.background.height != new_background.height:
			tkMessageBox.showerror("Hoodcraft", "The new background must be the same size as the existing one - the background is {0}x{1} and this image is {2}x{3}.".format(project.background.width, project.background.height, new_background.width, new_background.height))
			return
	project.background = new_background
	project.background_name = path.rsplit(os.sep, 1)[1]
	if not project.terrain_map:
		project.terrain_map = vector_map(project.background.width, project.background.height)
	update_bgimage_label()
	modified = True
	update_title()

def update_bgimage_label():
	bgimage_label.config(text = "Background image: " + project.background_name)

def browse_alpha(i):
	global modified
	def ret():
		global modified
		if editor:
			hide_terrain_editor()
		if not project.background:
			tkMessageBox.showerror("Hoodcraft", "You must choose a background image before adding layer images.")
			return
		path = localise_path(tkFileDialog.askopenfilename(
			parent = root,
			filetypes = image_filetypes))
		if not path:
			return
		try:
			new_image = PIL.Image.open(normalise_path(path))
		except Exception as ex:
			tkMessageBox.showerror("Hoodcraft", "Failed to open {0}: {1}".format(path, ex))
			return
		if new_image.width != project.background.width or new_image.height != project.background.height:
			tkMessageBox.showerror("Hoodcraft", "Layers must be the same size as the background - the background is {0}x{1} and this image is {2}x{3}.".format(project.background.width, project.background.height, new_image.width, new_image.height))
			return
		project.alpha[i] = new_image
		project.alpha_names[i] = path.rsplit(os.sep, 1)[1]
		update_alpha_label(i)
		modified = True
		update_title()
	return ret

def browse_opaque(i):
	global modified
	def ret():
		global modified
		if editor:
			hide_terrain_editor()
		if not project.background:
			tkMessageBox.showerror("Hoodcraft", "You must choose a background image before adding layer images.")
			return
		path = localise_path(tkFileDialog.askopenfilename(
			parent = root,
			filetypes = image_filetypes))
		if not path:
			return
		try:
			new_image = PIL.Image.open(normalise_path(path))
		except Exception as ex:
			tkMessageBox.showerror("Hoodcraft", "Failed to open {0}: {1}".format(path, ex))
			return
		if new_image.width != project.background.width or new_image.height != project.background.height:
			tkMessageBox.showerror("Hoodcraft", "Layers must be the same size as the background - the background is {0}x{1} and this image is {2}x{3}.".format(project.background.width, project.background.height, new_image.width, new_image.height))
			return
		project.opaque[i] = new_image
		project.opaque_names[i] = path.rsplit(os.sep, 1)[1]
		update_opaque_label(i)
		modified = True
		update_title()
	return ret

def clear_alpha(i):
	global modified
	def ret():
		if editor:
			hide_terrain_editor()
		if not project.alpha[i] or tkMessageBox.askyesno(
			"Hoodcraft", 
			"Are you sure you would like to clear transparent layer {0}?".format(i)) == "no":
			return
		project.alpha[i] = None
		project.alpha_names[i] = "(none)"
		update_alpha_label(i)
		modified = True
		update_title()
	return ret

def clear_opaque(i):
	global modified
	def ret():
		if editor:
			hide_terrain_editor()
		if not project.opaque[i] or tkMessageBox.askyesno(
			"Hoodcraft", 
			"Are you sure you would like to clear opaque layer {0}?".format(i)) == "no":
			return
		project.opaque[i] = None
		project.opaque_names[i] = "(none)"
		update_opaque_label(i)
		modified = True
		update_title()
	return ret

def update_alpha_label(i):
	layer_ctls[i][0].config(text = "Transparent Layer {0}: {1}".format(i + 1, project.alpha_names[i]))

def update_opaque_label(i):
	layer_ctls[i][1].config(text = "Opaque Layer {0}: {1}".format(i + 1, project.opaque_names[i]))

editor = None

class terrain_editor(object, Tkinter.Frame):
	select_mode = 0
	insert_mode = 1
	region_colours = ["#7f0000", "#007f00", "#00007f", "#7f7f00", "#007f7f"]
	def copy_real_polys(self):
		for iid, item in self.polys:
			self.canvas.delete(iid)
		for i, (line, item) in enumerate(project.terrain_map.regions):
			colour = self.region_colours[i % len(self.region_colours)]
			if item is self.selected:
				colour = colour.replace("7", "f")
			self.polys.append((self.canvas.create_polygon(line, fill = colour), item))
	def click(self, event):
		x = self.canvas.canvasx(event.x)
		y = self.canvas.canvasy(event.y)
		point = (x, y)
		if x > project.terrain_map.width or y > project.terrain_map.height:
			return
		if self.mode == terrain_editor.select_mode:
			self.selected = project.terrain_map.get_at(x, y)
			self.copy_real_polys()
			return
		if self.mode == terrain_editor.insert_mode:
			global modified
			if self.insert_first:
				if abs(self.insert_first[0] - x) + abs(self.insert_first[1] - y) < 15:
					(x, y) = point = self.insert_first
					project.terrain_map.add_region(self.insert_points, terrain_region())
					self.clear_insert()
					self.copy_real_polys()
					modified = True
					update_title()
				else:
					self.insert_lines.append(self.canvas.create_line((self.insert_points[len(self.insert_points) - 1], point), fill = "magenta"))
					self.insert_points.append(point)
			else:
				self.insert_first = point
				self.insert_points.append(point)
			return
	def clear_insert(self):
		for line in self.insert_lines:
			self.canvas.delete(line)
		self.insert_first = None
		self.insert_points = []
		self.insert_lines = []
	def selectmode(self):
		self.mode = terrain_editor.select_mode
		self.selectmodebtn.config(relief = Tkinter.SUNKEN)
		self.insertmodebtn.config(relief = Tkinter.RAISED)
		self.clear_insert()
	def insertmode(self):
		self.mode = terrain_editor.insert_mode
		self.selectmodebtn.config(relief = Tkinter.RAISED)
		self.insertmodebtn.config(relief = Tkinter.SUNKEN)
		self.selected = None
		self.copy_real_polys()
	def toggle_background(self, name, index, mode):
		self.canvas.itemconfigure(self.background_id, state = "normal" if self.var_bg.get() else "hidden")
	def toggle_polys(self, name, index, mode):
		for iid, item in self.polys:
			self.canvas.itemconfigure(iid, state = "normal" if self.var_polys.get() else "hidden")
	def change_altitude(self, name, index, mode):
		global modified
		if self.selected:
			try:
				self.selected.height = int(self.var_altitude.get())
				modified = True
				update_title()
			except ValueError:
				self.var_altitude.set(str(self.selected.height))
	def change_walkable(self, name, index, mode):
		global modified
		if self.selected:
			self.selected.walkable = bool(self.var_walkable.get())
			modified = True
			update_title()
	def change_lit(self, name, index, mode):
		global modified
		if self.selected:
			self.selected.lit = bool(self.var_lit.get())
			modified = True
			update_title()
	def toggle_alpha(self, i):
		return lambda name, index, mode: self.canvas.itemconfigure(self.alpha[i][1], state = "normal" if self.alpha[i][2].get() else "hidden")
	def toggle_opaque(self, i):
		return lambda name, index, mode: self.canvas.itemconfigure(self.opaque[i][1], state = "normal" if self.opaque[i][2].get() else "hidden")
	def send_forward(self):
		global modified
		project.terrain_map.swap_region(self.selected, 1)
		self.copy_real_polys()
		modified = True
		update_title()
	def send_backward(self):
		global modified
		project.terrain_map.swap_region(self.selected, -1)
		self.copy_real_polys()
		modified = True
		update_title()
	def delete_selected(self):
		global modified
		if tkMessageBox.askyesno("Hoodcraft", "Are you sure you want to delete this region?"):
			project.terrain_map.remove_region(self.selected)
			self.copy_real_polys()
			modified = True
			update_title()
	def __init__(self, *args, **kwargs):
		Tkinter.Frame.__init__(self, *args, **kwargs)
		self._selected = None
		self.toolbar = Tkinter.Frame(self)
		self.toolbar.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.mode = terrain_editor.select_mode
		self.selectmodebtn = Tkinter.Button(self.toolbar, text = "Select", command = self.selectmode, relief = Tkinter.SUNKEN)
		self.selectmodebtn.pack(side = Tkinter.LEFT)
		self.insertmodebtn = Tkinter.Button(self.toolbar, text = "Insert", command = self.insertmode)
		self.insertmodebtn.pack(side = Tkinter.LEFT)
		self.viewframe = Tkinter.Frame(self)
		self.viewframe.pack(side = Tkinter.LEFT, fill = Tkinter.BOTH, expand = True)
		self.settingsframe = Tkinter.Frame(self)
		self.settingsframe.pack(side = Tkinter.RIGHT, fill = Tkinter.BOTH)
		self.canvas = Tkinter.Canvas(self.viewframe, scrollregion = (0, 0, project.background.width, project.background.height))
		self.xscroll = Tkinter.Scrollbar(self.viewframe, command = self.canvas.xview, orient = Tkinter.HORIZONTAL)
		self.yscroll = Tkinter.Scrollbar(self.viewframe, command = self.canvas.yview)
		self.xscroll.pack(side = Tkinter.BOTTOM, fill = Tkinter.X)
		self.yscroll.pack(side = Tkinter.RIGHT, fill = Tkinter.Y)
		self.canvas.config(xscrollcommand = self.xscroll.set, yscrollcommand = self.yscroll.set)
		self.canvas.pack(fill = Tkinter.BOTH, expand = True)
		self.show_header = Tkinter.Label(self.settingsframe, text = "Show")
		self.show_header.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.background = PIL.ImageTk.PhotoImage(project.background)
		self.background_id = self.canvas.create_image(0, 0, anchor = Tkinter.NW, image = self.background)
		self.var_bg = Tkinter.IntVar(value = True)
		self.var_bg.trace("w", self.toggle_background)
		self.show_bg = Tkinter.Checkbutton(self.settingsframe, text = "Background", variable = self.var_bg)
		self.show_bg.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.alpha = [None] * 8
		self.opaque = [None] * 8
		for i in xrange(8):
			if project.alpha[i]:
				img = PIL.ImageTk.PhotoImage(project.alpha[i])
				iid = self.canvas.create_image(0, 0, anchor = Tkinter.NW, image = img)
				var = Tkinter.IntVar(value = True)
				var.trace("w", self.toggle_alpha(i))
				check = Tkinter.Checkbutton(self.settingsframe, text = "Alpha Layer {0}".format(i + 1), variable = var)
				check.pack(side = Tkinter.TOP, fill = Tkinter.X)
				self.alpha[i] = (img, iid, var, check)
			if project.opaque[i]:
				img = PIL.ImageTk.PhotoImage(project.opaque[i])
				iid = self.canvas.create_image(0, 0, anchor = Tkinter.NW, image = img)
				var = Tkinter.IntVar(value = True)
				var.trace("w", self.toggle_opaque(i))
				check = Tkinter.Checkbutton(self.settingsframe, text = "Opaque Layer {0}".format(i + 1), variable = var)
				check.pack(side = Tkinter.TOP, fill = Tkinter.X)
				self.opaque[i] = (img, iid, var, check)
		self.var_polys = Tkinter.IntVar(value = True)
		self.var_polys.trace("w", self.toggle_polys)
		self.show_polys = Tkinter.Checkbutton(self.settingsframe, text = "Terrain Map", variable = self.var_polys)
		self.show_polys.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.polys = []
		self.copy_real_polys()
		self.insert_points = []
		self.insert_lines = []
		self.insert_first = None
		self.edit_header = Tkinter.Label(self.settingsframe, text = "Edit")
		self.edit_header.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.var_altitude = Tkinter.StringVar(value = "0")
		self.var_altitude.trace("w", self.change_altitude)
		self.altitude_frame = Tkinter.Frame(self.settingsframe)
		self.altitude_frame.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.altitude_label = Tkinter.Label(self.altitude_frame, text = "Altitude:")
		self.altitude_label.pack(side = Tkinter.LEFT)
		self.altitude_box = Tkinter.Entry(self.altitude_frame, textvariable = self.var_altitude)
		self.altitude_box.pack(side = Tkinter.LEFT)
		self.var_walkable = Tkinter.IntVar(value = True)
		self.var_walkable.trace("w", self.change_walkable)
		self.walkable_check = Tkinter.Checkbutton(self.settingsframe, text = "Walkable", variable = self.var_walkable)
		self.walkable_check.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.var_lit = Tkinter.IntVar(value = False)
		self.var_lit.trace("w", self.change_lit)
		self.lit_check = Tkinter.Checkbutton(self.settingsframe, text = "Lit", variable = self.var_lit)
		self.lit_check.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.moveupbtn = Tkinter.Button(self.settingsframe, text = "Send Forward", command = self.send_forward)
		self.moveupbtn.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.movedownbtn = Tkinter.Button(self.settingsframe, text = "Send Backward", command = self.send_backward)
		self.movedownbtn.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.deletebtn = Tkinter.Button(self.settingsframe, text = "Delete", command = self.delete_selected)
		self.deletebtn.pack(side = Tkinter.TOP, fill = Tkinter.X)
		self.selected = None
		self.focus_set()
		self.bind("<Escape>", lambda e: self.clear_insert())
		self.bind("<Control-s>", lambda e: save())
		self.canvas.bind("<Button-1>", self.click)
	@property
	def selected(self):
		return self._selected
	@selected.setter
	def selected(self, value):
		self._selected = value
		if value:
			self.altitude_box.config(state = "normal")
			self.var_altitude.set(str(value.height))
			self.walkable_check.config(state = "normal")
			self.var_walkable.set(value.walkable)
			self.lit_check.config(state = "normal")
			self.var_lit.set(value.lit)
			self.moveupbtn.config(state = "normal")
			self.movedownbtn.config(state = "normal")
			self.deletebtn.config(state = "normal")
		else:
			self.altitude_box.config(state = "disabled")
			self.var_altitude.set("0")
			self.walkable_check.config(state = "disabled")
			self.var_walkable.set(False)
			self.lit_check.config(state = "disabled")
			self.var_lit.set(False)
			self.moveupbtn.config(state = "disabled")
			self.movedownbtn.config(state = "disabled")
			self.deletebtn.config(state = "disabled")

def hide_terrain_editor():
	global editor
	editor.destroy()
	editor = None

def show_terrain_editor():
	global editor
	if not project.terrain_map:
		tkMessageBox.showerror("Hoodcraft", "You must choose a background image before editing the terrain map.")
		return
	if editor:
		editor.focus_force()
		return
	editor = Tkinter.Toplevel(root)
	editor.title("Hoodcraft Terrain Editor")
	editor.protocol("WM_DELETE_WINDOW", hide_terrain_editor)
	terrain_editor(editor).pack(fill = Tkinter.BOTH, expand = True)

root = Tkinter.Tk()
update_title()
menubar = Tkinter.Menu(root)
root.config(menu = menubar)
file_menu = Tkinter.Menu(menubar)
menubar.add_cascade(label = "File", menu = file_menu)
file_menu.add_command(label = "New", command = new)
file_menu.add_command(label = "Open...", command = open_file)
file_menu.add_command(label = "Save", command = save)
file_menu.add_command(label = "Save As...", command = save_as)
file_menu.add_separator()
file_menu.add_command(label = "Build", command = build)
file_menu.add_separator()
file_menu.add_command(label = "Quit", command = verify_quit)

name_frame = Tkinter.Frame(root)
name_frame.pack(side = Tkinter.TOP, fill = Tkinter.X)
tk_hood_name = Tkinter.StringVar()
tk_hood_name.trace("w", update_project_name)
Tkinter.Label(name_frame, text = "Hood name:").pack(side = Tkinter.LEFT)
Tkinter.Entry(name_frame, textvariable = tk_hood_name).pack(fill = Tkinter.BOTH, expand = True)

bgimage_frame = Tkinter.Frame(root)
bgimage_frame.pack(side = Tkinter.TOP, fill = Tkinter.X)
bgimage_label = Tkinter.Label(bgimage_frame)
update_bgimage_label()
bgimage_label.pack(side = Tkinter.LEFT)
Tkinter.Button(bgimage_frame, text = "Browse", command = browse_bgimage).pack(side = Tkinter.LEFT)

layer_ctls = []
for i in xrange(8):
	rframe = Tkinter.Frame(root)
	rframe.pack(side = Tkinter.TOP, fill = Tkinter.X)
	oframe = Tkinter.Frame(rframe)
	oframe.pack(side = Tkinter.LEFT)
	olabel = Tkinter.Label(oframe)
	olabel.pack(side = Tkinter.LEFT)
	Tkinter.Button(oframe, text = "Browse", command = browse_opaque(i)).pack(side = Tkinter.LEFT)
	Tkinter.Button(oframe, text = "X", command = clear_opaque(i)).pack(side = Tkinter.LEFT)
	aframe = Tkinter.Frame(rframe)
	aframe.pack(side = Tkinter.RIGHT)
	alabel = Tkinter.Label(aframe)
	alabel.pack(side = Tkinter.LEFT)
	Tkinter.Button(aframe, text = "Browse", command = browse_alpha(i)).pack(side = Tkinter.LEFT)
	Tkinter.Button(aframe, text = "X", command = clear_alpha(i)).pack(side = Tkinter.LEFT)
	layer_ctls.append((alabel, olabel))
	update_alpha_label(i)
	update_opaque_label(i)

Tkinter.Button(root, text = "Edit Terrain Map", command = show_terrain_editor).pack(side = Tkinter.TOP, fill = Tkinter.X)

root.protocol("WM_DELETE_WINDOW", verify_quit)
root.bind("<Control-s>", lambda e: save())
root.mainloop()


