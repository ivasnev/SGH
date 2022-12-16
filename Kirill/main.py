from PyQt5 import QtWidgets, QtCore
import pyqtgraph as pg
import sys  # We need sys so that we can pass argv to QApplication

import serial
from time import sleep
import json

from form import Ui_MainWindow

from math import sin


class GraphUnit:
    def __init__(self, label, _max, _min, _size, _widget):
        self.data_size = _size
        self.label = label
        self.widget = _widget
        self.widget.setBackground([0, 0, 0, 0])
        self.x = list(range(self.data_size))
        self.y = [0 for _ in range(self.data_size)]
        self.y_min = [_min for _ in range(self.data_size)]
        self.y_max = [_max for _ in range(self.data_size)]
        self.widget.showAxis('bottom', False)

        pen = pg.mkPen(width=3, color=(50, 50, 200, 100))

        pen_max = pg.mkPen(width=3, color=(255, 0, 0, 200))
        pen_min = pg.mkPen(width=3, color=(0, 0, 255, 200))

        self.data_line = self.widget.plot(self.x, self.y, pen=pen, fillLevel=-0.3, brush=(50, 50, 200, 100))

        self.data_line_min = self.widget.plot(self.x, self.y_min, pen=pen_min)
        self.data_line_max = self.widget.plot(self.x, self.y_max, pen=pen_max)


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.Debug_mode = True
        self.tmp_x = 0
        self.port = 'COM5'
        self.arduino = None
        if not self.Debug_mode:
            self.connect_to_arduino()
        self.sensors = ["tem_air", "hum_air", "hum_soil", "water_lvl"]
        self.widgets = {}
        self.boarders = {
            "tem_air": [20, 80],
            "hum_air": [30, 70],
            "hum_soil": [40, 60],
            "water_lvl": [50, 55]
        }
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.graphs = {key: GraphUnit(key, *self.boarders[key], 100, self.widgets[key]) for key in self.sensors}
        self.timer = QtCore.QTimer()
        self.timer.setInterval(500)
        self.timer.timeout.connect(self.update_plot_data)
        self.timer.start()

    def connect_to_arduino(self):
        try:
            self.arduino = serial.Serial(self.port, 9600)
        except serial.SerialException:
            print("Нет подключения, переподключение через 5 секунд")
            sleep(5)
            self.connect_to_arduino()
        else:
            print("Wait for connection...")
            sleep(2)
            for _ in range(3):
                json_inp = str(self.arduino.readline())
                print(_, "test load", json_inp)
                sleep(1)

    def set_label(self):
        self.ui.max_hum_v.setText(str(self.boarders["hum_air"][1]))
        self.ui.min_hum_v.setText(str(self.boarders["hum_air"][0]))
        self.ui.max_temp_v.setText(str(self.boarders["tem_air"][1]))
        self.ui.min_temp_v.setText(str(self.boarders["tem_air"][0]))
        self.ui.max_hum_soil_v.setText(str(self.boarders["hum_soil"][1]))
        self.ui.min_hum_soil_v.setText(str(self.boarders["hum_soil"][0]))
        self.ui.max_water_v.setText(str(self.boarders["water_lvl"][1]))
        self.ui.min_water_v.setText(str(self.boarders["water_lvl"][0]))

    def get_data(self):
        __inp = str(self.arduino.readline())
        __inp = __inp[2:-5]
        __dic = json.loads(__inp)
        return __dic

    def update_plot_data(self):
        if self.Debug_mode:
            self.tmp_x += 0.3
            __data = {
                key: sin(self.tmp_x / 4) * 100 for key in self.sensors
            }
        else:
            __data = self.get_data()
        for key in self.sensors:
            self.graphs[key].x = self.graphs[key].x
            self.graphs[key].y = self.graphs[key].y[1:]  # Remove the first y element.
            self.graphs[key].y.append(__data[key])  # Add a new value.
            self.graphs[key].data_line.setData(self.graphs[key].x, self.graphs[key].y)  # Update the data.


app = QtWidgets.QApplication(sys.argv)
w = MainWindow()
w.show()
sys.exit(app.exec_())