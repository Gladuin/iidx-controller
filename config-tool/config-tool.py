import time
import threading

from tkinter import *
from tkinter import ttk
from tkinter import messagebox

from pywinusb import hid

def get_filtered_devices():
    filter = hid.HidDeviceFilter(vendor_id = 0x0001, product_id = 0x0001)
    return filter.get_devices()

def send(data):
    if len(get_filtered_devices()) == 0:
        messagebox.showwarning(title = "No controller", message = "Please connect a controller before\ntrying to send commands")
        return False

    device = get_filtered_devices()[0]
    device.open()

    report_data = bytearray(3)
    report_data[0] = 0x03
    report_data[1] = (data >> 8)
    report_data[2] = (data & 0xFF)

    device.send_output_report(report_data)
    device.close()
    return True

def send_config(controller_mode, led_mode, tt_mode, tt_inc, debounce_time, polling_rate, temporary_config):
    concat_hex = lambda a, b: a << 8 | b

    if send(concat_hex(0x01, int(temporary_config))) == False:
        return False

    send(concat_hex(0x10, controller_mode))
    send(concat_hex(0x20, led_mode))
    send(concat_hex(0x30, tt_mode))
    send(concat_hex(0x40, int(tt_inc)))
    send(concat_hex(0x50, int(debounce_time)))
    send(concat_hex(0x60, round(((1 / int(polling_rate)) * 1000))))
    send(0x01FF) # config send end

    return True

def validate_input(entry_input):
    if entry_input == "":
        return True
    elif entry_input.isdigit() == False:
        messagebox.showwarning(title = "Invalid input", message = "Only digits are allowed in the input box")
        return False
    elif int(entry_input) > 255 or int(entry_input) < 1:
        messagebox.showwarning(title = "Invalid input", message = "Only numbers between 1 and 255\nare allowed in the input box")
        return False
    else:
        return True

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
        controller_mode = ttk.Combobox(mainframe, values = ["Joystick", "Keyboard"])
        controller_mode.grid(column = 1, row = 2, sticky = W)
        controller_mode.state(["!disabled", "readonly"])
        controller_mode.current(0)

        ttk.Label(mainframe, text = "LED mode").grid(column = 0, row = 3, sticky = W)
        led_mode = ttk.Combobox(mainframe, values = ["Reactive + HID", "HID", "Reactive", "Off"])
        led_mode.grid(column = 1, row = 3, sticky = W)
        led_mode.state(["!disabled", "readonly"])
        led_mode.current(0)

        ttk.Label(mainframe, text = "TT mode").grid(column = 0, row = 4, sticky = W)
        tt_mode = ttk.Combobox(mainframe, values = ["Analog (joystick)", "Analog (mouse)", "Digital (joystick)", "Digital (button)", "Digital (keyboard)"])
        tt_mode.grid(column = 1, row = 4, sticky = W)
        tt_mode.state(["!disabled", "readonly"])
        tt_mode.current(0)

        ttk.Separator(mainframe, orient="horizontal").grid(column = 0, row = 5, columnspan = 2, sticky = (E, W))

        validate_command = (self.root.register(validate_input), "%P")

        tt_inc_var = StringVar()
        tt_inc_var.set("72")
        debounce_time_var = StringVar()
        debounce_time_var.set("5")

        ttk.Label(mainframe, text = "TT increments per full turn").grid(column = 0, row = 6, sticky = W)
        ttk.Entry(mainframe, validate = "key", validatecommand = validate_command, textvariable = tt_inc_var).grid(column = 1, row = 6, sticky = (E, W))

        ttk.Label(mainframe, text = "Debounce time (ms)").grid(column = 0, row = 7, sticky = W)
        ttk.Entry(mainframe, validate = "key", validatecommand = validate_command, textvariable = debounce_time_var).grid(column = 1, row = 7, sticky = (E, W))

        polling_rate_array = ["1000", "500", "250", "125"]

        ttk.Label(mainframe, text = "Polling rate (Hz)").grid(column = 0, row = 8, sticky = W)
        polling_rate = ttk.Combobox(mainframe, values = polling_rate_array)
        polling_rate.grid(column = 1, row = 8, sticky = W)
        polling_rate.state(["!disabled", "readonly"])
        polling_rate.current(0)

        ttk.Separator(mainframe, orient="horizontal").grid(column = 0, row = 9, columnspan = 2, sticky = (E, W))

        button_frame = ttk.Frame(mainframe)
        button_frame.grid(column = 0, row = 10, columnspan = 2, sticky = (N, W, E, S))

        ttk.Button(button_frame, text = "Reset controller", command = lambda:send(0xFF00)).grid(column = 0, row = 0, sticky = (E, W))
        ttk.Button(button_frame, text = "Save",
                   command = lambda:send_config(controller_mode.current(),
                                                led_mode.current(),
                                                tt_mode.current(),
                                                tt_inc_var.get(),
                                                debounce_time_var.get(),
                                                polling_rate_array[polling_rate.current()],
                                                False)
                  ).grid(column = 2, row = 0, sticky = (E, W))

        ttk.Button(button_frame, text = "Apply",
                   command = lambda:send_config(controller_mode.current(),
                             led_mode.current(),
                             tt_mode.current(),
                             tt_inc_var.get(),
                             debounce_time_var.get(),
                             polling_rate_array[polling_rate.current()],
                             True)
                  ).grid(column = 3, row = 0, sticky = (E, W))

        button_frame.grid_columnconfigure(1, weight = 1)

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