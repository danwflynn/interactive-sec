from picamera2 import Picamera2
import paho.mqtt.client as mqtt
import cv2
import numpy as np

BROKER = "localhost"
TOPIC = "drawing/coordinates"

def track_red_dot():
    picam2 = Picamera2()
    picam2.configure(picam2.create_preview_configuration(main={"size": (640, 480)}))
    picam2.start()

    client = mqtt.Client()
    client.connect(BROKER)

    while True:
        frame = picam2.capture_array()
        hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        lower_blue = np.array([125, 100, 50])
        upper_blue = np.array([145, 255, 255])
        blue_mask = cv2.inRange(hsv_frame, lower_blue, upper_blue)

        contours, _ = cv2.findContours(blue_mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        if contours:
            largest_contour = max(contours, key=cv2.contourArea)
            (x, y), radius = cv2.minEnclosingCircle(largest_contour)
            if radius > 10:
                client.publish(TOPIC, f"{x} {y}")
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

if __name__ == "__main__":
    track_red_dot()
