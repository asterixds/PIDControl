#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main()
{
  uWS::Hub h;

  PID pid;
  // TODO: Initialize the pid variable.
  pid.Init(0.2, 1e-4, 9.0);

  
  // pid for throttle
  PID pid_t;
  std::deque<double> steering_log;
  const int LOG_SIZE = 15;
  pid_t.Init(1, 1e-4, 5);



  h.onMessage([&pid,&pid_t,&steering_log](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          // steering log
          steering_log.push_front(angle);

          if (steering_log.size() > LOG_SIZE)
            steering_log.pop_back();

          double abs_avg_angle = std::fabs(std::accumulate(steering_log.begin(), steering_log.end(), 0.0) / steering_log.size());
            
          // setup desired speed
          double desired_speed= 50;
          //here we try to increase the speed if the average steering angle is not too sharp
          //the curve is chosen somewhat arbitrarily as a quadraic trendline which decreases
          //desired speed as angle increases
          if (speed > 15) {
            if (abs_avg_angle < 1.0)
              desired_speed = 100.0;
            else if (abs_avg_angle > 10.0)
              desired_speed = 16.0;
            else 
              desired_speed  = 0.6722*abs_avg_angle*abs_avg_angle - 15.807*abs_avg_angle + 107.82;
            //increase desired speed for low cte but slow down as it increase beyong 0.3 with a lower speed
            //limit of 10 for higher than 0.7 cte
            double acte = fabs(cte);
            if(acte < 0.7)
              desired_speed += 90.885 *acte *acte - 147.04 *acte + 33.939;
            else
              desired_speed = 16;// slow down
            desired_speed = std::fmin(desired_speed,100.0);
            desired_speed = std::fmax(desired_speed,16.0);

          }
          pid_t.UpdateError(speed-desired_speed);
          double throttle = pid_t.TotalError();
              
          pid.UpdateError(cte);
          steer_value = pid.TotalError();
          
          // DEBUG
          std::cout << "CTE: " << cte << " Steering Value: " << steer_value << std::endl;

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = throttle;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
