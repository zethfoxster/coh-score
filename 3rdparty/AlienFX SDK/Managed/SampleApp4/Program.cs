using System;
using LightFX;
using System.Threading;

namespace SampleApp4
{
	class Program
	{
		static void Main()
		{
			var lightFX = new LightFXController();

			var result = lightFX.LFX_Initialize();
			if (result == LFX_Result.LFX_Success)
			{
				for (int i = 0; i <= 255; i++)
				{
					var color = new LFX_ColorStruct(0xFF, (byte)(0xFF - i), (byte)i, 0x00);

					lightFX.LFX_Reset();
					lightFX.LFX_Light(LFX_Position.LFX_All, color);
					Console.Clear();
					Console.Write("Color:" + color);
					lightFX.LFX_Update();

					Thread.Sleep(150);
				}

				Console.WriteLine();
				Console.WriteLine("Done.\r\rPress ENTER key to finish ...");
				Console.ReadLine();
				lightFX.LFX_Release();
			}
			else
			{
				switch (result)
				{
					case LFX_Result.LFX_Error_NoDevs:
						Console.WriteLine("There is not AlienFX device available.");
						break;
					default:
						Console.WriteLine("There was an error initializing the AlienFX device.");
						break;
				}
			}
		}
	}
}
