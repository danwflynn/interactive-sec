import tkinter as tk
from tkinter import PhotoImage
from PIL import Image, ImageTk

# Create the main window
root = tk.Tk()
root.title("PNG Image on Canvas")

# Set up the canvas
canvas = tk.Canvas(root, width=1280, height=960)
canvas.pack()

# Open the image using PIL
image_path = "..\drawing.png"  # Replace with your image file path
image = Image.open(image_path)

# Convert the image to a Tkinter-compatible format
photo = ImageTk.PhotoImage(image)

# Display the image on the canvas
canvas.create_image(0, 0, anchor=tk.NW, image=photo)

# Start the Tkinter event loop
root.mainloop()
