#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>
#include <limits>

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

/**********************************/
 PID speed_control_pid = PID();
 PID steering_pid;
 int no_of_steps = 0;
 int pid_index = 0;
 double pid_array[10][3];
 pid_array[0][0] = 0.105312;  pid_array[0][1] = 0.0025747; pid_array[0][2] = 0.368219;
 pid_array[1][0] = 0.099312; pid_array[1][1]=0.0025747; pid_array[1][1] =0.368219; // best
//double Kp = 0.120312;  pid_array[0][1] = 0.0025747; pid_array[0][2] = 0.368219;
 pid_array[2][0] = 0.105312;  pid_array[2][1] = 0.0025747; pid_array[2][2] = 0.368219;
 pid_array[3][0] = 0.095312;  pid_array[3][1] = 0.0025747; pid_array[3][2] = 0.368219;
 pid_array[4][0] = 0.085312;  pid_array[4][1] = 0.0025747; pid_array[4][2] = 0.328219;
 pid_array[5][0] = 0.080312;  pid_array[5][1] = 0.0025747; pid_array[5][2] = 0.328219;
 pid_array[6][0] = 0.0803131;  pid_array[6][1] = 0.0025747; pid_array[6][2] = 0.328219;
 pid_array[7][0] = 0.080323;  pid_array[7][1] = 0.0025747; pid_array[7][2] = 0.328219;
 

 //double Kp = 0.105312; double Ki =0.0025747; double Kd =0.368219; // best 
 speed_control_pid.Init(.3, 0, 1.);
 steering_pid.Init(pid_array[0][0], pid_array[0][1] , pid_array[0][2] );
 
 h.onMessage([&](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
   

    if (length && length > 0.1 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") 
	{
		auto j = json::parse(s);
		std::string event = j[0].get<std::string>();


		if (event == "telemetry")
		 {
	             	no_of_steps++;
		     			
			// j[1] is the data JSON object
			double cte = std::stod(j[1]["cte"].get<std::string>());
			double speed = std::stod(j[1]["speed"].get<std::string>());
			double speed_error, speed_value, steer_value;
			double target_speed = 30;				        
			steering_pid.UpdateError(cte + 0.5);
			steer_value = steering_pid.TotalError();					
			speed_error =  speed - target_speed;
			speed_control_pid.UpdateError(speed_error);
			speed_value =  speed_control_pid.TotalError();
			json msgJson;
		   	msgJson["steering_angle"] = steer_value;
			if (fabs(cte) > 4.) 
			  { 	pid_index++; 
				steering_pid.Init(pid_array[pid_index][0], pid_array[pid_index][1] , pid_array[pid_index][2]);
				std::string msg("42[\"reset\", {}]");
				ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
			  }
			if (fabs(cte) > 2.) { speed_value = 0.3/(cte*cte);}
			if (no_of_steps < 10) { speed_value = no_of_steps/10.; steer_value = 0; } 
		    	msgJson["throttle"] = speed_value;
		   	//std::cout << " speed value "<< speed_value << std::endl;
			auto msg = "42[\"steer\"," + msgJson.dump() + "]";
		    	//std::cout << msg << std::endl;
		    	ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
			//std::cout << "cte: " << cte << "  no of steps   " << no_of_steps << std::endl;

		  }

        } 
	   else 
		{
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
