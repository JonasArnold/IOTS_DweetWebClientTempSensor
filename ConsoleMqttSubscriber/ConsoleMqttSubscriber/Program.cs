using System;
using System.Text;
using System.Threading;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;

namespace ConsoleMqttSubscriber
{
  class Program
  {

		static string ipMqttClient = "192.168.1.161";

		static void Main(string[] args)
		{
			Console.WriteLine("Hello MQTT World! Now listening to messages.");
			// creating an MqttClient object
			var client = new uPLibrary.Networking.M2Mqtt.MqttClient(ipMqttClient);
			// register to message received
			client.MqttMsgPublishReceived += client_MqttMsgPublishReceived;
			// generate a clientID and connect to Broker
			string clientId = Guid.NewGuid().ToString();
			client.Connect(clientId);
			// subscribe to a topic
			client.Subscribe(new string[] { "sensor1/light" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });
			client.Subscribe(new string[] { "sensor1/temperature" }, new byte[] { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE });

			// unused because publish is not required:
			// Endless loop
			//while (true)
			//{
			//	Thread.Sleep(50);
			//	if(Console.KeyAvailable)
			//     {
			//		var key = Console.ReadKey();
			//		if(key.Key == ConsoleKey.Spacebar)
			//       {
			//         Console.WriteLine($"Sending \"Test Message\" to topic 'scada/status'");
			//			client.Publish("scada/status", Encoding.ASCII.GetBytes("Test Message"));
			//       }
			//     }
			//}
		}
		static void client_MqttMsgPublishReceived(object sender, MqttMsgPublishEventArgs e)
		{
			// handle message received 
			Console.Write("Message received: ");
			Console.Write(Encoding.UTF8.GetString(e.Message) + "\n");
		}
	}
}
