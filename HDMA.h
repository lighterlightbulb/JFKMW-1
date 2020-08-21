#pragma once

void ResetDMAandHDMA()
{
	//Disable all HDMA and DMA channels.
	RAM[0x420C] = 0;
	RAM[0x420B] = 0;
}

void ProcessHDMA()
{
	/*
		Reset Horizontal Tables
	*/
	layer1mode_x = false;
	layer1mode_y = false;
	layer2mode_x = false;
	layer2mode_y = false;
	memset(layer1_shiftX, 0, sizeof(layer1_shiftX));
	memset(layer1_shiftY, 0, sizeof(layer1_shiftX));
	memset(layer2_shiftX, 0, sizeof(layer1_shiftX));
	memset(layer2_shiftY, 0, sizeof(layer1_shiftX));

	for (uint_fast8_t c = 0; c < 8; c++)
	{
		hdma_size[c] = 0;

		uint_fast8_t channel = c << 4;
		if ((RAM[0x420C] >> c) & 1) //This HDMA channel is enabled
		{
			
			uint_fast8_t mode = RAM[0x4300 + channel];
			uint_fast8_t size = mode + 1;
			uint_fast8_t reg = RAM[0x4301 + channel];
			uint_fast32_t bank = RAM[0x4302 + channel] + (RAM[0x4303 + channel] << 8) + (RAM[0x4304 + channel] << 16);
			uint_fast16_t i = 0;
			uint_fast16_t scanline = 0;

			if (reg == 0x0D) { layer1mode_x = true; }
			if (reg == 0x0E) { layer1mode_y = true; }
			if (reg == 0x0F) { layer2mode_x = true; }
			if (reg == 0x10) { layer2mode_y = true; }


			while (true)
			{
				uint_fast8_t scanlines = RAM[bank + i];
				if (scanlines == 0)
				{
					hdma_size[c]++;
					break;
				}
				else
				{
					for (uint_fast8_t l = 0; l < scanlines; l++)
					{
						int value = RAM[bank + 1 + i] + (size > 2 ? (RAM[bank + 2 + i] << 8) : 0);
						if (value >= 0x8000) { value = -(0x10000 - value); }

						if (scanline < 224)
						{
							if (reg == 0x0D) { layer1_shiftX[scanline] = int_fast16_t(value); }
							if (reg == 0x0E) { layer1_shiftY[scanline] = int_fast16_t(value); }
							if (reg == 0x0F) { layer2_shiftX[scanline] = int_fast16_t(value); }
							if (reg == 0x10) { layer2_shiftY[scanline] = int_fast16_t(value); }
						}
						scanline++;
					}
					i += size;
					hdma_size[c] += size;
				}
			}

		}
	}
}