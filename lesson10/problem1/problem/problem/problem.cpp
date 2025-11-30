#define _USE_MATH_DEFINES
#include "svg.h"

#include <cmath>
#include <sstream>
#include <cassert>

using namespace std::literals;
using namespace svg;

namespace {

    Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays) {
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
        }

        polyline.SetFillColor("red"s);
        polyline.SetStrokeColor("black"s);
        return polyline;
    }

    /*void DrawGreeting() {
        Document doc;
        doc.Add(Circle().SetCenter({ 20, 20 }).SetRadius(10));
        doc.Add(Text()
            .SetFontFamily("Verdana"s)
            .SetPosition({ 35, 20 })
            .SetOffset({ 0, 6 })
            .SetFontSize(12)
            .SetFontWeight("bold"s)
            .SetData("Hello C++"s));
        doc.Add(CreateStar({ 20, 50 }, 10, 5, 5));
        doc.Render(std::cout);
    }*/

}  // namespace

namespace shapes {

    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
            : p1_(p1)
            , p2_(p2)
            , p3_(p3) {
        }

        // Реализует метод Draw интерфейса svg::Drawable
        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
        }

    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
            :center_(center)
            , outer_rad_(outer_rad)
            , inner_rad_(inner_rad)
            , num_rays_(num_rays) {
        }

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_));
        }

    private:
        svg::Point center_;
        double outer_rad_;
        double inner_rad_;
        int num_rays_;
    };
    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point center, double rad)
            :center_(center)
            , rad_(rad) {
        }

        void Draw(svg::ObjectContainer& container) const override {
            container.Add(svg::Circle()
                .SetCenter({ center_.x, center_.y + (5 * rad_) })
                .SetRadius(rad_ * 2)
                .SetFillColor(Rgb{ 240,240,240 })
                .SetStrokeColor("black"s));
            container.Add(svg::Circle()
                .SetCenter({ center_.x, center_.y + (2 * rad_) })
                .SetRadius(rad_ * 1.5)
                .SetFillColor(Rgb{ 240,240,240 })
                .SetStrokeColor("black"s));
            container.Add(svg::Circle()
                .SetCenter(center_)
                .SetRadius(rad_)
                .SetFillColor(Rgb{ 240,240,240 })
                .SetStrokeColor("black"s));
        }

    private:
        svg::Point center_;
        double rad_;
    };

} // namespace shapes 


template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
    for (auto it = begin; it != end; ++it) {
        (*it)->Draw(target);
    }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
    using namespace std;
    DrawPicture(begin(container), end(container), target);
}

// Выполняет линейную интерполяцию значения от from до to в зависимости от параметра t.
uint8_t Lerp(uint8_t from, uint8_t to, double t) {
    return static_cast<uint8_t>(std::round((to - from) * t + from));
}

svg::Rgb Lerp(svg::Rgb from, svg::Rgb to, double t) {
    return { Lerp(from.red, to.red, t), Lerp(from.green, to.green, t), Lerp(from.blue, to.blue, t) };
}

void TestColor() {
    {
        Circle().SetFillColor(Rgb(1, 2, 3)).Render(std::cout);
    } {
        Circle().SetFillColor(Rgba(1, 2, 3, 0.5)).Render(std::cout);
    } {
        Circle().SetFillColor("black").Render(std::cout);
    } {
        Circle().SetFillColor("be").Render(std::cout);
    } {
        Circle().SetFillColor(NoneColor).Render(std::cout);
    }
}

int main() {
    //using namespace svg;
    //using namespace std;

    //Color none_color{};
    //cout << none_color << endl; // none

    //Color purple{ "purple"s };
    //cout << purple << endl; // purple

    //Color rgb = Rgb{ 100, 200, 255 };

    ////assert(get<Rgb>(rgb).red == 100);
    ////assert(get<Rgb>(rgb).green == 200);
    ////assert(get<Rgb>(rgb).blue == 255);
    //cout << rgb << endl; // rgb(100,200,255)

    //Color rgba = Rgba{ 100, 200, 255, 0.5 };
    //cout << rgba << endl; // rgba(100,200,255,0.5)

    //Circle c;
    //c.SetRadius(3.5).SetCenter({ 1.0, 2.0 });
    //c.SetFillColor(rgba);
    //c.SetStrokeColor(purple);

    //Document doc;
    //doc.Add(std::move(c));
    //doc.Render(cout);
    TestColor();

}
