from tkinter import *
from tkinter import ttk

controller_connected_text = "No controller connected!"
controller_connected_color = "red"

root = Tk()
root.title("config-tool")

mainframe = ttk.Frame(root, padding = (3, 3, 3, 3))
mainframe.grid(column = 0, row = 0, sticky = (N, W, E, S))
root.columnconfigure(0, weight = 1)
root.rowconfigure(0, weight = 1)

ttk.Label(mainframe, text = controller_connected_text, foreground = controller_connected_color).grid(column = 0, row = 0, columnspan = 2)


ttk.Separator(mainframe, orient="horizontal").grid(column = 0, row = 1, columnspan = 2, sticky = (E, W))


ttk.Label(mainframe, text = "Controller mode").grid(column = 0, row = 2, sticky = W)
controllermode = ttk.Combobox(mainframe, values = ["test", "test"])
controllermode.grid(column = 1, row = 2, sticky = W)
controllermode.state(["!disabled", "readonly"])
controllermode.current(0)

ttk.Label(mainframe, text = "LED mode").grid(column = 0, row = 3, sticky = W)
ledmode = ttk.Combobox(mainframe, values = ["test", "test"])
ledmode.grid(column = 1, row = 3, sticky = W)
ledmode.state(["!disabled", "readonly"])
ledmode.current(0)

radiobuttonframe = ttk.Frame(mainframe)
radiobuttonframe.grid(column = 1, row = 4, sticky = (N, W, E, S))

ttk.Label(mainframe, text = "TT mode").grid(column = 0, row = 4, sticky = W)
ttk.Radiobutton(radiobuttonframe, text = "Digital").grid(column = 0, row = 0, sticky = W, padx = (0, 5))
ttk.Radiobutton(radiobuttonframe, text = "Analog").grid(column = 1, row = 0, sticky = W)


ttk.Separator(mainframe, orient="horizontal").grid(column = 0, row = 5, columnspan = 2, sticky = (E, W))


ttk.Label(mainframe, text = "TT increments per full turn").grid(column = 0, row = 6, sticky = W)
ttk.Entry(mainframe).grid(column = 1, row = 6, sticky = (E, W))

ttk.Label(mainframe, text = "Debounce time").grid(column = 0, row = 7, sticky = W)
ttk.Entry(mainframe).grid(column = 1, row = 7, sticky = (E, W))

ttk.Label(mainframe, text = "Polling rate").grid(column = 0, row = 8, sticky = W)
pollingrate = ttk.Combobox(mainframe, values = ["test", "test"])
pollingrate.grid(column = 1, row = 8, sticky = W)
pollingrate.state(["!disabled", "readonly"])
pollingrate.current(0)


ttk.Separator(mainframe, orient="horizontal").grid(column = 0, row = 9, columnspan = 2, sticky = (E, W))


buttonframe = ttk.Frame(mainframe)
buttonframe.grid(column = 0, row = 10, columnspan = 2, sticky = (N, W, E, S))

ttk.Button(buttonframe, text = "Reset controller").grid(column = 0, row = 0, sticky = (E, W))
ttk.Button(buttonframe, text = "Save").grid(column = 2, row = 0, sticky = (E, W))
ttk.Button(buttonframe, text = "Apply").grid(column = 3, row = 0, sticky = (E, W))

buttonframe.grid_columnconfigure(1, weight = 1)


for child in mainframe.winfo_children(): 
    child.grid_configure(padx=2, pady=2)

root.mainloop()