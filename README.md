# CarND-Controls-PID
THe objective of this project is to drive the car in the simulator by calculating the steer angle using a PID controller. I used 2 controllers, one for the steering and the other for the throttle control.

The gain settings kp, ki, kd components were arrived at using a set of experiments increasing or decreasing each parameter individually whilst keeping the others constant.

The final gain setting that gave satisfctory results was:
Kp = 0.2, Ki= 1e-4 and Kd = 9.0

The following observations contributed to the final settings:
* A larger value for Kp caused higher oscillation and the car went off the driving track at the curves.
* As Ki is based on the integral of the error, larger value towards 1.0 results in little forward movement and makes it longer for the car to get back on track. 
* Lower values of Kd resulted in underdamping and the car would zigzag around.


For the throttle controller, the error is the difference between the desired speed and the actual speed. The desired speed is set using a hueristic which increases the speed if the steering angle is close to zero and decreases it gradually as the steering angle (an average of past several readings) increases (as the car goes round the bends). The desired speed was similarly adjusted for base cross track error. 

The gradual adjustments were approximated by the quadratic curves - these were fitted from step adjustments.
* desired_speed  = 0.6722*abs_avg_angle*abs_avg_angle - 15.807*abs_avg_angle + 107.82
* desired_speed += 90.885 *acte *acte - 147.04 *acte + 33.939

If the cte or ateering angle is too high, the desired speed is set to a low value chosen somewhat arbitrarily at 16mph.

The results can be seen at  [video](https://youtu.be/n5ICjTL0oqg) where the car can be seen sticking to the road and slowing down at the curves, accelerating as the road straightens.

------------------------
## Basic Build Instructions
1. Clone this repo.
2. Make a build directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./pid`. 

