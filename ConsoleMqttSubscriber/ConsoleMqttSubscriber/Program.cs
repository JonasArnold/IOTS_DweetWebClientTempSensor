using System;
using System.Text;
using System.Threading;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace ConsoleMqttSubscriber
{
  class Program
  {
    static string ipMqttClient = "test.mosquitto.org";
    static bool ledOnState = false;

    static void Main(string[] args)
    {
      // creating an MqttClient object
      var client = new uPLibrary.Networking.M2Mqtt.MqttClient(ipMqttClient);
      Console.WriteLine($"Connected to MQTT broker at: {ipMqttClient}");
      Console.WriteLine("Hello MQTT World! Now listening to messages. " +
        "\nPress Spacebar to toggle LED on/off." +
        "\nPress Enter to configure a new cycle time (ms).");
      // register to message received
      client.MqttMsgPublishReceived += client_MqttMsgPublishReceived;
      // generate a clientID and connect to Broker
      string clientId = Guid.NewGuid().ToString();
      client.Connect(clientId);
      // subscribe to a topic
      client.Subscribe(new string[] { "sensor1/light" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
      client.Subscribe(new string[] { "sensor1/temperature" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });

      // Endless loop
      while (true)
      {
        Thread.Sleep(50);
        if (Console.KeyAvailable)
        {
          var key = Console.ReadKey();
          if (key.Key == ConsoleKey.Spacebar)
          {
            string stateMessage = "ON";
            if (ledOnState)
            {
              stateMessage = "OFF";
            }

            string ledToggleTopic = "sensor1/ledState";
            Console.WriteLine($"Sending \"{stateMessage}\" to topic '{ledToggleTopic}'");
            client.Publish(ledToggleTopic, Encoding.ASCII.GetBytes(stateMessage));
            ledOnState = !ledOnState;  // toggle
          }
          else if (key.Key == ConsoleKey.Enter)
          {
            Console.Write("Please provide cycle time in [ms]: ");
            var readLine = Console.ReadLine();
            int cycleTime = Convert.ToInt32(readLine);
            if(cycleTime < 60000 && cycleTime > 2000)
            {
              string cycleTimeTopic = "sensor1/cylceTime";
              Console.WriteLine($"Sending \"{cycleTime}\" to topic '{cycleTimeTopic}'");
              client.Publish(cycleTimeTopic, Encoding.ASCII.GetBytes(cycleTime.ToString()));
            }
            else
            {
              Console.WriteLine("Cycle time invalid. Was not sent!");
            }
          }
        }
      }
    }

    static void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
    {
      // handle message received 
      Console.Write("Message received: ");
      Console.Write(Encoding.UTF8.GetString(e.Message) + "\n");
    }
  }
}
