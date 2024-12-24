import os
import sys
import serial
from datetime import datetime
from PyQt5.QtWidgets import (QApplication, QMainWindow, QGridLayout, QWidget, 
                             QPushButton, QComboBox, QLabel, QLineEdit, QMessageBox)
from PyQt5.QtGui import QColor
import pyqtgraph as pg
from PyQt5.QtCore import QTimer, QThread, pyqtSignal

class DataReaderThread(QThread):
    data_received = pyqtSignal(int, int)  # Thay đổi tín hiệu để truyền 2 giá trị

    def __init__(self, serial_port, baud_rate, parent=None):
        super(DataReaderThread, self).__init__(parent)
        self.ser = serial.Serial(serial_port, baud_rate)
        self.paused = False

    def run(self):
        while True:
            if not self.paused:
                data = self.ser.read(4)  # Read 4 bytes
                if len(data) == 4:
                    value1 = (data[0] << 8) + data[1]
                    value2 = (data[2] << 8) + data[3]
                    self.data_received.emit(value1, value2)  # Emit both channels


class RealTimePlot(QMainWindow):
    def __init__(self):
        super(RealTimePlot, self).__init__()
        self.initUI()

    def initUI(self):
        # Set up the main window
        self.setGeometry(100, 100, 1280, 720)
        self.setWindowTitle('Real-Time Plot with PyQt5 and pyqtgraph')

        # Set up the central widget and layout
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)
        layout = QGridLayout(central_widget)

        # Create a pyqtgraph PlotWidget
        self.plot_widget = pg.PlotWidget()
        self.plot_widget.setBackground('w')
        layout.addWidget(self.plot_widget, 0, 1, 1, 2)

        # Customize the plot
        self.plot_widget.setYRange(1000, 3000)
        self.plot_widget.showGrid(x=True, y=True, alpha=0.5)
        self.plot_widget.getPlotItem().getAxis('left').setPen(pg.mkPen(color='k'))
        self.plot_widget.getPlotItem().getAxis('bottom').setPen(pg.mkPen(color='k'))
        self.plot_widget.getPlotItem().getAxis('left').setTextPen(pg.mkPen(color='k'))
        self.plot_widget.getPlotItem().getAxis('bottom').setTextPen(pg.mkPen(color='k'))
        self.plot_widget.getPlotItem().getViewBox().setBackgroundColor('w')

        # Set up the plot
        self.plot = self.plot_widget.plot(pen=pg.mkPen(color='k', width=2))
        self.max_data_points = 1000
        self.circular_buffer = [0] * self.max_data_points
        self.circular_index = 0

        # Thread for data reading
        self.thread = None

        # Set button dimensions
        button_width = 200
        button_height = 30

        self.com_port_combo = QComboBox(self)
        self.baud_rate_combo = QComboBox(self)

        # Text fields
        self.patient_edit = QLineEdit(self)      # Patient name
        self.patient_id_edit = QLineEdit(self)   # Patient ID
        self.type_edit = QLineEdit(self)         # Data type
        self.record_time_combo = QComboBox(self)

        # Thời gian đo mặc định là 30 giây
        record_times = ['30', '60', '90', '180', '300','3']
        self.record_time_combo.addItems(record_times)
        self.record_time_combo.setCurrentText('60')  # đặt mặc định 60 giây

        self.start_button = QPushButton('Start', self)
        self.stop_button = QPushButton('Stop', self)
        self.pause_button = QPushButton('Pause', self)
        self.record_button = QPushButton('Record', self)

        # Adjust sizes
        self.start_button.setFixedSize(button_width, button_height)
        self.stop_button.setFixedSize(button_width, button_height)
        self.pause_button.setFixedSize(button_width, button_height)
        self.record_button.setFixedSize(button_width, button_height)
        line_edit_width = 300
        combo_box_width = 300

        self.patient_edit.setFixedWidth(line_edit_width)
        self.patient_id_edit.setFixedWidth(line_edit_width)
        self.type_edit.setFixedWidth(line_edit_width)

        self.com_port_combo.setFixedWidth(combo_box_width)
        self.baud_rate_combo.setFixedWidth(combo_box_width)
        self.record_time_combo.setFixedWidth(combo_box_width)

        # Labels
        self.com_label = QLabel('COM Port:', self)
        self.baud_rate_label = QLabel('Baud Rate:', self)
        self.patient_label = QLabel('Patient:', self)
        self.patient_id_label = QLabel('Patient ID:', self)
        self.type_label = QLabel('Type:', self)
        self.record_time_label = QLabel('Record Time:', self)

        # Populate COM ports and Baud rates
        self.populate_com_ports()
        baud_rates = ['9600', '14400', '19200', '38400', '57600', '115200', '230400']
        self.baud_rate_combo.addItems(baud_rates)
        self.baud_rate_combo.setCurrentText('115200')  # đặt mặc định 60 giây

        # Connect buttons
        self.start_button.clicked.connect(self.start_reading)
        self.stop_button.clicked.connect(self.stop_reading)
        self.pause_button.clicked.connect(self.toggle_pause)
        self.record_button.clicked.connect(self.record_data)

        # Button colors
        self.start_button.setStyleSheet("background-color: green")
        self.stop_button.setStyleSheet("background-color: red")
        self.pause_button.setStyleSheet("background-color: yellow")
        self.record_button.setStyleSheet("background-color: blue")

        # Add widgets to layout
        row_index = 1
        layout.addWidget(self.com_label, row_index, 0)
        layout.addWidget(self.com_port_combo, row_index, 1)
        row_index += 1

        layout.addWidget(self.baud_rate_label, row_index, 0)
        layout.addWidget(self.baud_rate_combo, row_index, 1)
        row_index += 1

        layout.addWidget(self.patient_label, row_index, 0)
        layout.addWidget(self.patient_edit, row_index, 1)
        row_index += 1

        layout.addWidget(self.patient_id_label, row_index, 0)
        layout.addWidget(self.patient_id_edit, row_index, 1)
        row_index += 1

        layout.addWidget(self.type_label, row_index, 0)
        layout.addWidget(self.type_edit, row_index, 1)
        row_index += 1

        layout.addWidget(self.record_time_label, row_index, 0)
        layout.addWidget(self.record_time_combo, row_index, 1)
        row_index += 1

        layout.addWidget(self.start_button, row_index, 0)
        layout.addWidget(self.stop_button, row_index, 1)
        row_index += 1

        layout.addWidget(self.pause_button, row_index, 0)
        layout.addWidget(self.record_button, row_index, 1)

        # QTimer for plot update
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_plot)

        # Variables
        self.paused = False
        self.recording = False
        self.recorded_data = []

    def populate_com_ports(self):
        from serial.tools.list_ports import comports
        com_ports = [port.device for port in comports()]
        self.com_port_combo.clear()
        self.com_port_combo.addItems(com_ports)

    def start_reading(self):
        if self.thread is None or not self.thread.isRunning():
            com_port = self.com_port_combo.currentText()
            baud_rate = int(self.baud_rate_combo.currentText())
            patient = self.patient_edit.text()
            patient_id = self.patient_id_edit.text()
            data_type = self.type_edit.text()
            record_time = int(self.record_time_combo.currentText())

            print(f'Patient: {patient}, Patient ID: {patient_id}, Type: {data_type}, Record Time: {record_time} seconds')

            # Check required fields
            if not patient or not patient_id or not data_type:
                QMessageBox.warning(self, 'Notification', 'Please fill in all required fields.')
                return

            self.thread = DataReaderThread(com_port, baud_rate)
            self.thread.data_received.connect(self.update_plot)
            self.thread.start()
            self.timer.start(1000)

            # Reset recording status and data
            self.recording = False
            self.recorded_data = []

    def stop_reading(self):
        if self.thread is not None and self.thread.isRunning():
            # Check if recording is ongoing
            if self.recording:
                QMessageBox.warning(self, 'Notification', 'Recording stopped (without saving).')

            self.timer.stop()
            self.thread.terminate()
            if self.thread.ser.is_open:
                self.thread.ser.close()

    def toggle_pause(self):
        if self.thread is not None and self.thread.isRunning():
            self.thread.paused = not self.thread.paused
            self.paused = not self.paused
            if self.paused:
                self.pause_button.setText('Play')
            else:
                self.pause_button.setText('Pause')

    def record_data(self):
        if self.thread is not None and self.thread.isRunning() and not self.paused:
            self.recording = not self.recording

            if self.recording:
                QMessageBox.information(self, 'Notification', 'Recording started.')
                self.recorded_data = []
                QTimer.singleShot(int(self.record_time_combo.currentText()) * 1000, self.stop_recording)
            else:
                self.save_recorded_data()

    def stop_recording(self):
        if self.recording:
            self.recording = False
            self.save_recorded_data()

    def save_recorded_data(self):
        data_type = self.type_edit.text()
        patient = self.patient_edit.text()
        patient_id = self.patient_id_edit.text()

        # Check required fields
        if not patient or not patient_id or not data_type:
            QMessageBox.warning(self, 'Notification', 'Please fill in all required fields.')
            return

        # Tạo cấu trúc folder: patient-patientID/type
        directory_name = os.path.join(f"{patient}-{patient_id}", data_type)
        if not os.path.exists(directory_name):
            os.makedirs(directory_name, exist_ok=True)

        # Lấy thời gian hiện tại
        current_time_str = datetime.now().strftime("%Y%m%d_%H%M%S")

        # Tên file: patient_patientId_type_YYYYMMDD_HHMMSS.csv
        filename = os.path.join(directory_name, f'{patient}_{patient_id}_{data_type}_{current_time_str}.csv')

        # Check if the file already exists
        if os.path.exists(filename):
            reply = QMessageBox.question(self, 'Notification', 
                                        'The file already exists. Do you want to replace it?',
                                        QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
            if reply == QMessageBox.No:
                return

        # Ghi dữ liệu vào file CSV
        with open(filename, 'w') as file:
            file.write('Raw Data,Kalman Data\n')  # Header
            for value1, value2 in self.recorded_data:
                file.write(f'{value1},{value2}\n')  # Ghi cả hai channel

        QMessageBox.information(self, 'Notification', 'Recording stopped. Data was saved.')


    def update_plot(self, value1=None, value2=None):
        if value1 is not None and value2 is not None:
            self.circular_buffer[self.circular_index] = value2
            self.circular_index = (self.circular_index + 1) % self.max_data_points

            # Add the second channel value to another buffer or process accordingly
            if self.recording:
                self.recorded_data.append((value1, value2))  # Lưu cả 2 channel

        # Cập nhật đồ thị chỉ hiển thị channel 1 (có thể sửa để hiển thị cả 2 channel)
        self.plot.setData(self.circular_buffer)


    def closeEvent(self, event):
        self.stop_reading()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    mainWindow = RealTimePlot()
    mainWindow.show()
    sys.exit(app.exec_())  

