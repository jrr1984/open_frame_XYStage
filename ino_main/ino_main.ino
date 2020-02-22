void setup()
{ 
  Serial.begin(115200);
  connect_to_stage();
  comm_interface(); 
}

void loop()
{   
  run_in_loop();
}
