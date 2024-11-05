from picamera2 import Picamera2
import cv2
import numpy as np

def track_red_dot():
    picam2 = Picamera2()
    picam2.configure(picam2.create_preview_configuration(main={"size": (640, 480)}))
    picam2.start()

    while True:
        frame = picam2.capture_array()

        hsv_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        lower_blue = np.array([125, 100, 50])
        upper_blue = np.array([145, 255, 255])
        blue_mask = cv2.inRange(hsv_frame, lower_blue, upper_blue)
        
        blue_output = cv2.bitwise_and(frame, frame, mask=blue_mask)

        contours, _ = cv2.findContours(blue_mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        if contours:
            largest_contour = max(contours, key=cv2.contourArea)
            (x, y), radius = cv2.minEnclosingCircle(largest_contour)
            center = (int(x), int(y))
            radius = int(radius)

            if radius > 10:
                cv2.circle(frame, center, radius, (0, 255, 0), 2)
                cv2.circle(frame, center, 5, (0, 255, 0), -1)
        
        cv2.imshow("Red Dot Tracking", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cv2.destroyAllWindows()

if __name__ == "__main__":
    track_red_dot()
