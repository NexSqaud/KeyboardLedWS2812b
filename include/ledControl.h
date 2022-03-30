#include <stdint.h>
#include <cmath>
#include <vector>
#include <Adafruit_NeoPixel.h>

struct Color
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

struct Pixel
{
    int Line;
    int Pixel;
    Color PixelColor;
};

template <int LinesCount = 6>
class LedControl;

struct LinePacket
{
    int LineIndex;
    Color FillColor;

    LinePacket(int lineIndex, Color color) : LineIndex(lineIndex), FillColor(color)
    {}

    template <int LinesCount>
    void Apply(LedControl<LinesCount>& ledControl)
    {
        ledControl.FillLine(LineIndex, FillColor);
    }
};

struct PixelsPacket
{
    std::vector<Pixel> Pixels;

    PixelsPacket(std::vector<Pixel>& pixels) : Pixels(pixels)
    {}

    template <int LinesCount>
    void Apply(LedControl<LinesCount>& ledControl)
    {
        for (int i = 0; i < Pixels.size(); i++)
        {
            auto& pixel = Pixels[i];
            ledControl.SetPixel(pixel.Line, pixel.Pixel, pixel.PixelColor);
        }
    }
};

struct FillPacket
{
    Color FillColor;

    FillPacket(Color color) : FillColor(color)
    {}

    template <int LinesCount>
    void Apply(LedControl<LinesCount>& ledControl)
    {
        ledControl.Fill(FillColor);
    }
};

struct ClearPacket
{
    ClearPacket()
    {}

    template <int LinesCount>
    void Apply(LedControl<LinesCount>& ledControl)
    {
        ledControl.Fill(Color{ .Red = 0, .Green = 0, .Blue = 0 });
    }
};

struct SettingsPacket
{
    uint8_t Brightness;

    SettingsPacket(uint8_t brightness) : Brightness(brightness)
    {}

    template <int LinesCount>
    void Apply(LedControl<LinesCount>& ledControl)
    {
        ledControl.SetBrightness(Brightness);
    }
};

struct LedControlState {
    uint8_t Type;
    int EffectIndex;
    int ArraySize;
    std::vector<Pixel> Pixels;
};

template <int LinesCount>
class LedControl
{
    static void InitLine(Adafruit_NeoPixel& line, int lineIndex)
    {
        line.begin();
        line.clear();
        line.setBrightness(64);
        line.show();
    }

public:
    LedControl(std::array<Adafruit_NeoPixel, LinesCount>& lines)
    {
        Lines = lines;
        IterateLines(InitLine);
    }

    void SetPixel(int line, int pixel, Color color)
    {
        if (line >= LinesCount)
        {
            return;
        }

        if (pixel >= Lines[line].numPixels())
        {
            return;
        }

        Lines[line].setPixelColor(pixel, color.Red, color.Green, color.Blue);
        Lines[line].show();
    }

    void FillLine(int line, Color color)
    {
        if (line >= LinesCount)
        {
            return;
        }

        for (int i = 0; i < Lines[line].numPixels(); i++)
        {
            Lines[line].setPixelColor(i, color.Red, color.Green, color.Blue);
        }

        Lines[line].show();
    }

    void Fill(Color color)
    {
        IterateLines(
            [color](Adafruit_NeoPixel& line, int lineIndex)
            {
                for (int i = 0; i < line.numPixels(); i++)
                {
                    line.setPixelColor(i, color.Red, color.Green, color.Blue);
                }

                line.show();
            }
        );
    }


    void ClearPixel(int line, int pixel)
    {
        if (line >= LinesCount)
        {
            return;
        }

        if (pixel >= Lines[line].numPixels())
        {
            return;
        }

        Lines[line].setPixelColor(pixel, 0, 0, 0);
        Lines[line].show();
    }

    void ClearLine(int line)
    {
        if (line >= LinesCount)
        {
            return;
        }

        Lines[line].clear();
        Lines[line].show();
    }

    void Clear()
    {
        IterateLines(
            [](Adafruit_NeoPixel& line, int lineIndex)
            {
                line.clear();
                line.show();
            }
        );
    }

    void SetBrightness(int brightness)
    {
        IterateLines(
            [brightness](Adafruit_NeoPixel& line, int lineIndex)
            {
                line.setBrightness(brightness);
                line.show();
            }
        );
    }

private:

    void IterateLines(std::function<void(Adafruit_NeoPixel&, int)> iterator)
    {
        for (int i = 0; i < LinesCount; i++)
        {
            iterator(Lines[i], i);
        }
    }
    std::array<Adafruit_NeoPixel, LinesCount> Lines;

    int Brightness;
};