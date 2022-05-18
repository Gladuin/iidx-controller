import threading
import time

from tkinter import *
from tkinter import ttk

from pywinusb import hid

def get_filtered_devices():
    filter = hid.HidDeviceFilter(vendor_id = 0x0001, product_id = 0x0001)
    return filter.get_devices()

def send(data):
    device = get_filtered_devices()[0]
    device.open()

    report_data = bytearray(3)
    report_data[0] = 0x03
    report_data[1] = (data >> 8)
    report_data[2] = (data & 0xFF)

    device.send_output_report(report_data)
    device.close()

class GUI_CLASS(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self.start()

    def callback(self):
        self.root.quit()

    def run(self):
        global status_label

        self.root = Tk()
        self.root.protocol("WM_DELETE_WINDOW", self.callback)
        self.root.title("config-tool")

        mainframe = ttk.Frame(self.root, padding = (3, 3, 3, 3))
        mainframe.grid(column = 0, row = 0, sticky = (N, W, E, S))
        self.root.columnconfigure(0, weight = 1)
        self.root.rowconfigure(0, weight = 1)

        status_label = ttk.Label(mainframe)
        status_label.grid(column = 0, row = 0, columnspan = 2)

        ttk.Separator(mainframe, orient="horizontal").grid(column = 0, row = 1, columnspan = 2, sticky = (E, W))

        ttk.Label(mainframe, text = "Controller mode").grid(column = 0, row = 2, sticky = W)
        controllermode = ttk.Combobox(mainframe, values = ["Joystick", "Keyboard"])
        controllermode.grid(column = 1, row = 2, sticky = W)
        controllermode.state(["!disabled", "readonly"])
        controllermode.current(0)

        ttk.Label(mainframe, text = "LED mode").grid(column = 0, row = 3, sticky = W)
        ledmode = ttk.Combobox(mainframe, values = ["Reactive + HID", "HID", "Reactive", "Off"])
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

        ttk.Label(mainframe, text = "Debounce time (ms)").grid(column = 0, row = 7, sticky = W)
        ttk.Entry(mainframe).grid(column = 1, row = 7, sticky = (E, W))

        ttk.Label(mainframe, text = "Polling rate (Hz)").grid(column = 0, row = 8, sticky = W)
        pollingrate = ttk.Combobox(mainframe, values = ["1000", "500", "250", "125"])
        pollingrate.grid(column = 1, row = 8, sticky = W)
        pollingrate.state(["!disabled", "readonly"])
        pollingrate.current(0)

        ttk.Separator(mainframe, orient="horizontal").grid(column = 0, row = 9, columnspan = 2, sticky = (E, W))

        buttonframe = ttk.Frame(mainframe)
        buttonframe.grid(column = 0, row = 10, columnspan = 2, sticky = (N, W, E, S))

        ttk.Button(buttonframe, text = "Reset controller", command = lambda:send(0xFF00)).grid(column = 0, row = 0, sticky = (E, W))
        ttk.Button(buttonframe, text = "Save").grid(column = 2, row = 0, sticky = (E, W))
        ttk.Button(buttonframe, text = "Apply").grid(column = 3, row = 0, sticky = (E, W))

        buttonframe.grid_columnconfigure(1, weight = 1)

        for child in mainframe.winfo_children():
            child.grid_configure(padx=2, pady=2)

        self.root.mainloop()


gui = GUI_CLASS()

while gui.is_alive():
    if len(get_filtered_devices()) > 0:
        status_label.config(text = "Controller connected")
        status_label.config(foreground = "green")
    else:
        status_label.config(text = "Controller disconnected")
        status_label.config(foreground = "red")

    time.sleep(1)

exit()
