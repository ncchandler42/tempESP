#!/usr/bin/env python3

import tkinter as tk
import numpy as np
import cv2 as cv
import cv2.ml as ml

class MainScreen():
	def __init__(self, root):
		self.root = root		
		self.root.title("ML Colors")

		self.build_window()
		self.next_color()

		# TODO Initialize model if it doesn't already exist
		try:
			self.mlp = ml.ANN_MLP_load("model.yml")
		except:
			self.mlp = ml.ANN_MLP_create()

			# TODO Layer sizes, must start and end with 3 and 1
			layers = np.array([3, 6, 9, 6, 1])
			self.mlp.setLayerSizes(layers)

			# TODO Configure/initialize model. According to documentation, these are the default values
			self.mlp.setTermCriteria((cv.TERM_CRITERIA_MAX_ITER + cv.TERM_CRITERIA_EPS, 1000, 1e-2))
			self.mlp.setActivationFunction(ml.ANN_MLP_SIGMOID_SYM, 1, 1)
			self.mlp.setTrainMethod(ml.ANN_MLP_RPROP)

			self.mlp.save("model.yml")

	def __del__(self):
		self.mlp.save("model.yml")

	def build_window(self):
		# blah blah blah UI code, skip this...
		self.canvas = tk.Canvas(self.root,
			width=250, height=250)

		self.canvas.pack(side="left")

		################################################################
		self.bigframe = tk.Frame(self.root)
		
		self.uiframe = tk.LabelFrame(self.bigframe, text="Rate the color:")
		self.slider = tk.Scale(self.uiframe, 
			from_=0.0, to=10.0,
			orient=tk.HORIZONTAL, resolution=0.1, length=175
		)
			
		self.skip_b = tk.Button(self.uiframe, text="Skip", command=self.skip)
		self.train_b = tk.Button(self.uiframe, text="Train", command=self.train)
		
		self.slider.pack(side="top")
		self.train_b.pack(side="right")
		self.skip_b.pack(side="left")

		self.uiframe.pack(expand=True)
		
		##################################################################
		self.pframe = tk.Frame(self.bigframe)

		self.predict_b = tk.Button(self.pframe, text="Predict", command=self.predict)
		self.predict_tb = tk.Label(self.pframe)

		self.predict_b.pack(side="top")
		self.predict_tb.pack(side="bottom")

		self.pframe.pack(side="bottom", pady=25)
		self.bigframe.pack(side="right")
		
	def next_color(self):
		# TODO Color is a 1x1x3 float64 mat, range [0,1) (a.k.a. 1x1 img)
		self.colorHSV = np.random.random([1, 1, 3])
		self.color_inp = self.colorHSV[0].astype(np.float32) * 2 - 1 # range [0, 1) to (-1, 1)

		# convert to RGB, OpenCV expects [0, 360] for H, float32 image
		colorRGB = cv.cvtColor((self.colorHSV*np.array([360,1,1])).astype(np.float32), cv.COLOR_HSV2RGB)

		# update UI color
		hexcolor = "#{:02X}{:02X}{:02X}".format(*(int(n*255) for n in colorRGB[0,0]))
		self.canvas.delete("all")
		self.canvas.create_rectangle(0,0, 250, 250, fill=hexcolor)

		self.predict_tb.configure(text="")

	def train(self):
		# TODO Create training data from current color and chosen rating, 
		# could have multiple rows with more samples
		rating = self.slider.get() / 5 - 1
		
		td = ml.TrainData_create(
			self.color_inp,
			ml.ROW_SAMPLE,
			np.array([rating], dtype=np.float32)
		)

		# No automatic scaling, already taken care of
		if self.mlp.isTrained():
			self.mlp.train(td, ml.ANN_MLP_UPDATE_WEIGHTS | ml.ANN_MLP_NO_INPUT_SCALE | ml.ANN_MLP_NO_OUTPUT_SCALE)
		else:
			self.mlp.train(td, ml.ANN_MLP_NO_INPUT_SCALE | ml.ANN_MLP_NO_OUTPUT_SCALE)

		self.next_color()

	def skip(self):
		self.next_color()

	def predict(self):
		if not self.mlp.isTrained():
			self.predict_tb.configure(text="Not enough data.")
			return
		
		# TODO Gives prediction of current color. Returns a tuple, which doesn't match the documentation
		# 1x1 matrix should be the output
		_, premat = self.mlp.predict(self.color_inp)

		rating = premat[0,0] * 5 + 5
		self.predict_tb.configure(text=f"{rating:.1f}")

if __name__ == "__main__":
	root = tk.Tk()
	app = MainScreen(root)
	root.mainloop()
	
