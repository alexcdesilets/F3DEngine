#pragma once

#include "base.h"    // Assuming this includes <cmath>, <algorithm>, <array>, <vector>, <format>, <random>, <string>, <stdexcept>, <numbers>
// Standard C++ library includes needed:
#include <cstdint>   // For uint8_t
#include <iomanip>   // For std::setw, std::setfill, std::hex
#include <sstream>   // For std::ostringstream
#include <stdexcept> // For std::invalid_argument, std::out_of_range
#include <vector>    // For std::vector constructor
#include <algorithm> // For std::max, std::min, std::clamp, std::swap
#include <cmath>     // For safe_sqrt, std::round, std::fmod, std::floor, std::abs, std::atan2, std::acos, std::sin, std::cos, std::hypot, std::isinf, std::isnan, std::isfinite, std::copysign
#include <array>     // For std::array constructor and toArray()
#include <string>    // For std::string, std::stoul
#include <random>    // For std::mt19937_64, std::uniform_real_distribution, std::random_device

namespace vecSys {

    struct vecRGBA {
        double r = 0.0, g = 0.0, b = 0.0, a = 1.0; // Alpha defaults to 1.0 (opaque)

        // --- Constructors ---
        vecRGBA() {} // Default: Black, Opaque (0,0,0,1)
        
        // Constructor for grayscale with alpha
        vecRGBA(const double& grayscaleVal, const double& alphaVal = 1.0) 
            : r(grayscaleVal), g(grayscaleVal), b(grayscaleVal), a(alphaVal) {}

        vecRGBA(const double& redVal, const double& greenVal, const double& blueVal, const double& alphaVal = 1.0)
            : r(redVal), g(greenVal), b(blueVal), a(alphaVal) {}
        
        // Constructor from an array of 3 (RGB), alpha defaults to 1.0
        vecRGBA(const std::array<double, 3>& rgbChannels, const double& alphaVal = 1.0)
            : r(rgbChannels[0]), g(rgbChannels[1]), b(rgbChannels[2]), a(alphaVal) {}

        // Constructor from an array of 4 (RGBA)
        vecRGBA(const std::array<double, 4>& rgbaChannels)
            : r(rgbaChannels[0]), g(rgbaChannels[1]), b(rgbaChannels[2]), a(rgbaChannels[3]) {}

        // Constructor from std::vector<double>
        explicit vecRGBA(const std::vector<double>& vals) {
            if (vals.empty()) {
                r = 0.0; g = 0.0; b = 0.0; a = 1.0; // Default: Black, Opaque
            } else if (vals.size() == 1) { 
                r = vals[0]; g = vals[0]; b = vals[0]; a = 1.0; // Grayscale, Opaque
            } else if (vals.size() == 2) { // Interpreted as (R, G), B=0, A=1.0
                r = vals[0]; g = vals[1]; b = 0.0; a = 1.0;    
            } else if (vals.size() == 3) { // Interpreted as (R, G, B), A=1.0
                r = vals[0]; g = vals[1]; b = vals[2]; a = 1.0; 
            } else { // vals.size() >= 4, interpreted as (R,G,B,A)
                r = vals[0]; g = vals[1]; b = vals[2]; a = vals[3]; 
            }
        }
        
        vecRGBA(const vecRGBA& other) = default; // Default copy constructor

        // --- Static Factory Methods ---
        static constexpr vecRGBA Black(double alphaVal = 1.0) {return {0.0,0.0,0.0, alphaVal};}
        static constexpr vecRGBA DarkGrey(double alphaVal = 1.0) {return {0.25,0.25,0.25, alphaVal};}
        static constexpr vecRGBA Grey(double alphaVal = 1.0) {return {0.5,0.5,0.5, alphaVal};}
        static constexpr vecRGBA LightGrey(double alphaVal = 1.0) {return {0.75,0.75,0.75, alphaVal};}
        static constexpr vecRGBA White(double alphaVal = 1.0) {return {1.0,1.0,1.0, alphaVal};}
        
        static constexpr vecRGBA Red(double alphaVal = 1.0) {return {1.0,0.0,0.0, alphaVal};}
        static constexpr vecRGBA Green(double alphaVal = 1.0) {return {0.0,1.0,0.0, alphaVal};}
        static constexpr vecRGBA Blue(double alphaVal = 1.0) {return {0.0,0.0,1.0, alphaVal};}
        
        static constexpr vecRGBA Yellow(double alphaVal = 1.0) {return {1.0,1.0,0.0, alphaVal};}
        static constexpr vecRGBA Magenta(double alphaVal = 1.0) {return {1.0,0.0,1.0, alphaVal};}
        static constexpr vecRGBA Cyan(double alphaVal = 1.0) {return {0.0,1.0,1.0, alphaVal};}
        
        static constexpr vecRGBA Orange(double alphaVal = 1.0) {return {1.0, 0.647, 0.0, alphaVal};} 
        static constexpr vecRGBA Purple(double alphaVal = 1.0) {return {0.5, 0.0, 0.5, alphaVal};} 
        static constexpr vecRGBA Brown(double alphaVal = 1.0) {return {0.647, 0.165, 0.165, alphaVal};}
        static constexpr vecRGBA Pink(double alphaVal = 1.0) {return {1.0, 0.753, 0.796, alphaVal};}

        static constexpr vecRGBA TransparentBlack() {return {0.0,0.0,0.0,0.0};}
        static constexpr vecRGBA TransparentWhite() {return {1.0,1.0,1.0,0.0};}

        // Random color generation (instance method called by static factories)
        vecRGBA& randomize(double minR = 0.0, double maxR = 1.0, 
                           double minG = 0.0, double maxG = 1.0, 
                           double minB = 0.0, double maxB = 1.0, 
                           double minA = 1.0, double maxA = 1.0) {
            static std::mt19937_64 rng{std::random_device{}()};
            if (minR > maxR) std::swap(minR, maxR);
            if (minG > maxG) std::swap(minG, maxG);
            if (minB > maxB) std::swap(minB, maxB);
            if (minA > maxA) std::swap(minA, maxA);

            std::uniform_real_distribution<double> distR(minR, maxR);
            std::uniform_real_distribution<double> distG(minG, maxG);
            std::uniform_real_distribution<double> distB(minB, maxB);
            std::uniform_real_distribution<double> distA(minA, maxA);
            
            r = distR(rng); g = distG(rng); b = distB(rng); a = distA(rng);
            return *this;
        }

        static vecRGBA random(double minValR = 0.0, double maxValR = 1.0,
                              double minValG = 0.0, double maxValG = 1.0,
                              double minValB = 0.0, double maxValB = 1.0,
                              double minValA = 1.0, double maxValA = 1.0) {
            vecRGBA color;
            return color.randomize(minValR, maxValR, minValG, maxValG, minValB, maxValB, minValA, maxValA);
        }
        static vecRGBA randomOpaque(double minValRGB = 0.0, double maxValRGB = 1.0) {
            vecRGBA color;
            return color.randomize(minValRGB, maxValRGB, minValRGB, maxValRGB, minValRGB, maxValRGB, 1.0, 1.0);
        }
        static vecRGBA randomWithAlpha(double minValRGB = 0.0, double maxValRGB = 1.0, 
                                       double minValA = 0.0, double maxValA = 1.0) {
            vecRGBA color;
            return color.randomize(minValRGB, maxValRGB, minValRGB, maxValRGB, minValRGB, maxValRGB, minValA, maxValA);
        }

        // --- Core Color Operations ---
        // Opposite color (inverts RGB, alpha remains unchanged). Clamps RGB before inverting.
        vecRGBA oppositeRGB() const { 
            vecRGBA clampedColor = clamped(); 
            return {1.0 - clampedColor.r, 1.0 - clampedColor.g, 1.0 - clampedColor.b, this->a}; 
        }

        // Average color with another color (averages R,G,B,A components)
        vecRGBA average(const vecRGBA& other) const { 
            return { (r + other.r) * 0.5, 
                     (g + other.g) * 0.5, 
                     (b + other.b) * 0.5,
                     (a + other.a) * 0.5 }; 
        }

        // Modulate: Component-wise multiplication (r1*r2, g1*g2, b1*b2, a1*a2)
        vecRGBA modulate(const vecRGBA& other) const { 
            return {r * other.r, g * other.g, b * other.b, a * other.a}; 
        }

        // Luminance of the RGB part (perceptual brightness). Clamps RGB before calculation.
        double luminance() const { 
            vecRGBA c = clamped(); 
            return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b; 
        }

        // Grayscale conversion of RGB part, alpha is preserved.
        vecRGBA grayscale() const {
            double gray = luminance();
            return {gray, gray, gray, this->a};
        }

        // Euclidean distance in RGBA color space.
        double colorDistance(const vecRGBA& other) const { return (*this - other).magnitude(); }
        
        // Euclidean distance of only RGB components.
        double colorDistanceRGB(const vecRGBA& other) const {
            double dr = r - other.r;
            double dg = g - other.g;
            double db = b - other.b;
            return safe_sqrt(dr*dr + dg*dg + db*db);
        }

        // --- Color Space Conversions (operate on RGB, preserve alpha) ---
        // Returns vecRGBA: R=Hue, G=Saturation, B=Value, A=original Alpha.
        // Assumes RGB components are typically in the range [0,1]; clamps them internally.
        vecRGBA toHSV() const {
            vecRGBA clampedColor = clamped(); 
            double R = clampedColor.r, G = clampedColor.g, B = clampedColor.b;

            double maxC = std::max({R, G, B});
            double minC = std::min({R, G, B});
            double delta = maxC - minC;

            double h = 0.0, s = 0.0, v = maxC;

            if (delta > 0.0) { 
                if (maxC > 0.0) { 
                    s = delta / maxC;
                } else { 
                    s = 0.0; h = 0.0; 
                    return {h, s, v, this->a}; 
                }

                if (std::abs(maxC - R) == 0.0) { 
                    h = (G - B) / delta; 
                } else if (std::abs(maxC - G) == 0.0) {
                    h = 2.0 + (B - R) / delta; 
                } else { 
                    h = 4.0 + (R - G) / delta; 
                }
                h /= 6.0; 
                if (h < 0.0) { h += 1.0; }
            } else { 
                s = 0.0; h = 0.0; 
            }
            return {h, s, v, this->a}; 
        }
        
        static vecRGBA fromHSVA(double h, double s, double v, double alphaVal = 1.0) {
            h = std::fmod(h, 1.0); if (h < 0.0) h += 1.0;
            s = std::clamp(s, 0.0, 1.0);
            v = std::clamp(v, 0.0, 1.0);
            alphaVal = std::clamp(alphaVal, 0.0, 1.0);

            if (s == 0.0) { 
                return {v, v, v, alphaVal};
            }

            int i = static_cast<int>(std::floor(h * 6.0));
            double f = (h * 6.0) - static_cast<double>(i);
            double p = v * (1.0 - s);
            double q = v * (1.0 - s * f);
            double t = v * (1.0 - s * (1.0 - f));

            switch (i % 6) {
                case 0: return {v, t, p, alphaVal};
                case 1: return {q, v, p, alphaVal};
                case 2: return {p, v, t, alphaVal};
                case 3: return {p, q, v, alphaVal};
                case 4: return {t, p, v, alphaVal};
                case 5: return {v, p, q, alphaVal};
                default: return {0.0, 0.0, 0.0, alphaVal}; 
            }
        }

        vecRGBA toHSL() const {
            vecRGBA clampedColor = clamped();
            double R = clampedColor.r, G = clampedColor.g, B = clampedColor.b;

            double maxC = std::max({R, G, B});
            double minC = std::min({R, G, B});
            double delta = maxC - minC;

            double h = 0.0, s = 0.0, l = (maxC + minC) * 0.5;

            if (delta > 0.0) { 
                if (l == 0.0 || l == 1.0) { 
                    s = 0.0;
                } else {
                    s = (l < 0.5) ? (delta / (maxC + minC)) : (delta / (2.0 - maxC - minC));
                }

                if (std::abs(maxC - R) == 0.0) { 
                    h = (G - B) / delta + (G < B ? 6.0 : 0.0);
                } else if (std::abs(maxC - G) == 0.0) {
                    h = (B - R) / delta + 2.0;
                } else { 
                    h = (R - G) / delta + 4.0;
                }
                h /= 6.0; 
            } 
            return {h, s, l, this->a}; 
        }

        static vecRGBA fromHSLA(double h, double s, double l, double alphaVal = 1.0) {
            h = std::fmod(h, 1.0); if (h < 0.0) h += 1.0;
            s = std::clamp(s, 0.0, 1.0);
            l = std::clamp(l, 0.0, 1.0);
            alphaVal = std::clamp(alphaVal, 0.0, 1.0);

            if (s == 0.0) { 
                return {l, l, l, alphaVal};
            } else {
                auto hueToChannel = [](double p_val, double q_val, double t_val) -> double {
                    if (t_val < 0.0) t_val += 1.0;
                    if (t_val > 1.0) t_val -= 1.0;
                    if (t_val < 1.0 / 6.0) return p_val + (q_val - p_val) * 6.0 * t_val;
                    if (t_val < 1.0 / 2.0) return q_val;
                    if (t_val < 2.0 / 3.0) return p_val + (q_val - p_val) * (2.0 / 3.0 - t_val) * 6.0;
                    return p_val;
                };
                double q_val = (l < 0.5) ? (l * (1.0 + s)) : (l + s - l * s);
                double p_val = 2.0 * l - q_val;
                return {hueToChannel(p_val, q_val, h + 1.0/3.0), 
                        hueToChannel(p_val, q_val, h), 
                        hueToChannel(p_val, q_val, h - 1.0/3.0), 
                        alphaVal};
            }
        }

        // --- Integer and Hex String Conversions ---
        [[nodiscard]] std::array<uint8_t, 4> toRGBA8() const {
            vecRGBA c = clamped(); 
            return { static_cast<uint8_t>(std::round(c.r * 255.0)),
                     static_cast<uint8_t>(std::round(c.g * 255.0)),
                     static_cast<uint8_t>(std::round(c.b * 255.0)),
                     static_cast<uint8_t>(std::round(c.a * 255.0)) };
        }

        static vecRGBA fromRGBA8(uint8_t r_byte, uint8_t g_byte, uint8_t b_byte, uint8_t a_byte) {
            return { static_cast<double>(r_byte) / 255.0,
                     static_cast<double>(g_byte) / 255.0,
                     static_cast<double>(b_byte) / 255.0,
                     static_cast<double>(a_byte) / 255.0 };
        }
        static vecRGBA fromRGBA8(const std::array<uint8_t, 4>& rgba_bytes) {
            return fromRGBA8(rgba_bytes[0], rgba_bytes[1], rgba_bytes[2], rgba_bytes[3]);
        }

        [[nodiscard]] std::string toHexString(bool includeHash = true, bool includeAlpha = true) const {
            std::array<uint8_t, 4> rgba8 = toRGBA8();
            std::ostringstream oss;
            if (includeHash) oss << "#";
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[0]);
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[1]);
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[2]);
            if (includeAlpha) {
                oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[3]);
            }
            return oss.str();
        }

        static vecRGBA fromHexString(std::string hexStr) {
            if (hexStr.empty()) throw std::invalid_argument("Hex string cannot be empty.");
            if (hexStr[0] == '#') hexStr.erase(0, 1);

            if (hexStr.length() != 6 && hexStr.length() != 8) {
                throw std::invalid_argument("Hex string must be 6 (RGB) or 8 (RGBA) characters long (excluding optional '#').");
            }
            try {
                uint8_t r_byte = static_cast<uint8_t>(std::stoul(hexStr.substr(0, 2), nullptr, 16));
                uint8_t g_byte = static_cast<uint8_t>(std::stoul(hexStr.substr(2, 2), nullptr, 16));
                uint8_t b_byte = static_cast<uint8_t>(std::stoul(hexStr.substr(4, 2), nullptr, 16));
                uint8_t a_byte = (hexStr.length() == 8) ? static_cast<uint8_t>(std::stoul(hexStr.substr(6, 2), nullptr, 16)) : 255u;
                return fromRGBA8(r_byte, g_byte, b_byte, a_byte);
            } catch (const std::exception& e) { 
                throw std::invalid_argument("Invalid hex string format or value: " + hexStr + ". Details: " + e.what());
            }
        }

        // --- Alpha Compositing ---
        vecRGBA compositeOver(const vecRGBA& background) const {
            vecRGBA src = this->clamped(); 
            vecRGBA bg = background.clamped();

            double outA = src.a + bg.a * (1.0 - src.a);
            if (outA == 0.0) { 
                return {0.0, 0.0, 0.0, 0.0};
            }
            double invOutA = 1.0 / outA; // Precompute for efficiency
            double outR = (src.r * src.a + bg.r * bg.a * (1.0 - src.a)) * invOutA;
            double outG = (src.g * src.a + bg.g * bg.a * (1.0 - src.a)) * invOutA;
            double outB = (src.b * src.a + bg.b * bg.a * (1.0 - src.a)) * invOutA;
            
            return {outR, outG, outB, outA};
        }

        // --- Blending Modes ---
        vecRGBA blendMultiply(const vecRGBA& other) const {
            vecRGBA baseC = clamped(); vecRGBA otherC = other.clamped();
            return {baseC.r*otherC.r, baseC.g*otherC.g, baseC.b*otherC.b, a}; 
        }
        vecRGBA blendScreen(const vecRGBA& other) const {
            vecRGBA baseC = clamped(); vecRGBA otherC = other.clamped();
            return {1.0-(1.0-baseC.r)*(1.0-otherC.r), 1.0-(1.0-baseC.g)*(1.0-otherC.g), 1.0-(1.0-baseC.b)*(1.0-otherC.b), a};
        }
        vecRGBA blendOverlay(const vecRGBA& other) const {
            vecRGBA baseC = clamped(); vecRGBA otherC = other.clamped();
            auto comp = [](double bc, double oc){ return (bc<0.5)?(2.0*bc*oc):(1.0-2.0*(1.0-bc)*(1.0-oc)); };
            return {comp(baseC.r,otherC.r), comp(baseC.g,otherC.g), comp(baseC.b,otherC.b), a};
        }
        vecRGBA blendAdditive(const vecRGBA& other) const {
            vecRGBA temp = {r + other.r, g + other.g, b + other.b, a + other.a}; // Additive often sums alpha too
            return temp.clamp(0.0, 1.0); // Clamp all components
        }
        vecRGBA blendSubtract(const vecRGBA& other) const {
            vecRGBA temp = {r - other.r, g - other.g, b - other.b, a}; // Alpha from base
            return temp.clamp(0.0, 1.0); // Clamp RGB, alpha already from base
        }

        // --- Color Transformations ---
        vecRGBA adjustHue(double hueShift) const {
            vecRGBA hsv = toHSV(); 
            hsv.r = std::fmod(hsv.r + hueShift, 1.0); if(hsv.r < 0.0) hsv.r += 1.0;
            return fromHSVA(hsv.r, hsv.g, hsv.b, hsv.a); 
        }
        vecRGBA adjustSaturation(double satDelta) const {
            vecRGBA hsv = toHSV(); hsv.g = std::clamp(hsv.g + satDelta, 0.0, 1.0);
            return fromHSVA(hsv.r, hsv.g, hsv.b, hsv.a);
        }
        vecRGBA adjustBrightness(double brightDelta) const { 
            vecRGBA hsv = toHSV(); hsv.b = std::clamp(hsv.b + brightDelta, 0.0, 1.0);
            return fromHSVA(hsv.r, hsv.g, hsv.b, hsv.a);
        }
        vecRGBA adjustContrast(double factor) const {
            double f = std::max(0.0, factor); vecRGBA c = clamped(); 
            return {std::clamp(0.5+f*(c.r-0.5),0.0,1.0), std::clamp(0.5+f*(c.g-0.5),0.0,1.0), std::clamp(0.5+f*(c.b-0.5),0.0,1.0), a}; 
        }
        vecRGBA adjustAlpha(double alphaDelta) const { return {r,g,b,std::clamp(a+alphaDelta,0.0,1.0)}; }
        vecRGBA setAlpha(double newAlpha) const { return {r,g,b,std::clamp(newAlpha,0.0,1.0)}; }

        // --- "Vector-like" Operations (Treating RGBA as coordinates in 4D space) ---
        vecRGBA& round() { r=std::round(r);g=std::round(g);b=std::round(b);a=std::round(a); return *this; }
        [[nodiscard]] vecRGBA rounded() const { return {std::round(r),std::round(g),std::round(b),std::round(a)}; }

        vecRGBA& clamp(const double& minVal = 0.0, const double& maxVal = 1.0) {
            r=std::clamp(r,minVal,maxVal);g=std::clamp(g,minVal,maxVal);b=std::clamp(b,minVal,maxVal);a=std::clamp(a,minVal,maxVal); return *this;
        }
        [[nodiscard]] vecRGBA clamped(const double& minVal = 0.0, const double& maxVal = 1.0) const {
            return {std::clamp(r,minVal,maxVal),std::clamp(g,minVal,maxVal),std::clamp(b,minVal,maxVal),std::clamp(a,minVal,maxVal)};
        }
        vecRGBA& clamp(const vecRGBA& minV, const vecRGBA& maxV) { 
            r=std::clamp(r,minV.r,maxV.r);g=std::clamp(g,minV.g,maxV.g);b=std::clamp(b,minV.b,maxV.b);a=std::clamp(a,minV.a,maxV.a); return *this;
        }
        [[nodiscard]] vecRGBA clamped(const vecRGBA& minV, const vecRGBA& maxV) const {
            return {std::clamp(r,minV.r,maxV.r),std::clamp(g,minV.g,maxV.g),std::clamp(b,minV.b,maxV.b),std::clamp(a,minV.a,maxV.a)};
        }
        vecRGBA& clampMagnitude(double minL, double maxL) {
            double lSq = magnitudeSquared(); if(lSq==0.0)return *this; double l=safe_sqrt(lSq), sF=1.0;
            if(l<minL&&minL>0.0)sF=safe_divide(minL,l); else if(l>maxL&&maxL>=0.0)sF=safe_divide(maxL,l);
            if(std::abs(sF-1.0)>default_Tolerance&&sF!=0.0){r*=sF;g*=sF;b*=sF;a*=sF;}else if(sF==0.0&&maxL==0.0){r=0;g=0;b=0;a=0;} return *this;
        }
        [[nodiscard]] vecRGBA clampedMagnitude(double minL, double maxL) const { vecRGBA c=*this; return c.clampMagnitude(minL,maxL); }

        double dot(const vecRGBA& o) const { return r*o.r+g*o.g+b*o.b+a*o.a; }
        bool isFinite() const { return std::isfinite(r)&&std::isfinite(g)&&std::isfinite(b)&&std::isfinite(a); }
        bool isInfinite() const { return std::isinf(r)||std::isinf(g)||std::isinf(b)||std::isinf(a); }
        bool isNaN() const { return std::isnan(r)||std::isnan(g)||std::isnan(b)||std::isnan(a); }
        bool isZero() const { return r==0.0&&g==0.0&&b==0.0&&a==0.0; }
        bool isTransparent() const { return a == 0.0; }
        bool isOpaque() const { return a == 1.0; }

        double magnitude() const { if(isInfinite())return infinity; return safe_sqrt(r*r+g*g+b*b+a*a); }
        double magnitudeSquared() const { if(isInfinite())return infinity; return r*r+g*g+b*b+a*a; }

        vecRGBA& lerp(const vecRGBA& target, const double& amount) {
            double t=std::clamp(amount,0.0,1.0); 
            r=std::lerp(r,target.r,t);g=std::lerp(g,target.g,t);b=std::lerp(b,target.b,t);a=std::lerp(a,target.a,t); return *this;
        }
        [[nodiscard]] vecRGBA lerped(const vecRGBA& target, const double& amount) const { vecRGBA c=*this; return c.lerp(target,amount); }

        vecRGBA& setMax(const vecRGBA& o){r=std::max(r,o.r);g=std::max(g,o.g);b=std::max(b,o.b);a=std::max(a,o.a);return *this;}
        [[nodiscard]] vecRGBA max(const vecRGBA& o)const{return {std::max(r,o.r),std::max(g,o.g),std::max(b,o.b),std::max(a,o.a)};}
        vecRGBA& setMin(const vecRGBA& o){r=std::min(r,o.r);g=std::min(g,o.g);b=std::min(b,o.b);a=std::min(a,o.a);return *this;}
        [[nodiscard]] vecRGBA min(const vecRGBA& o)const{return {std::min(r,o.r),std::min(g,o.g),std::min(b,o.b),std::min(a,o.a)};}

        vecRGBA& moveTowards(const vecRGBA& target, double maxDist) {
            if(maxDist<=0.0)return *this; vecRGBA diff=target-*this; double dSq=diff.magnitudeSquared();
            if(dSq==0.0||dSq<=maxDist*maxDist){*this=target;}else{double d=safe_sqrt(dSq);*this+=(diff/d)*maxDist;} return *this;
        }
        [[nodiscard]] vecRGBA movedTowards(const vecRGBA& target,double maxDist)const{vecRGBA c=*this;return c.moveTowards(target,maxDist);}

        vecRGBA& normalize() {
            if(isZero())return *this; if(isInfinite()){
                r=(std::isinf(r)?std::copysign(1.0,r):0.0);g=(std::isinf(g)?std::copysign(1.0,g):0.0);
                b=(std::isinf(b)?std::copysign(1.0,b):0.0);a=(std::isinf(a)?std::copysign(1.0,a):0.0);
                double tl=magnitude(); if(tl>0.0){r/=tl;g/=tl;b/=tl;a/=tl;}else{r=1;g=0;b=0;a=0;} return *this;
            } double l=magnitude(); if(l==0.0)return *this; r/=l;g/=l;b/=l;a/=l; return *this;
        }
        [[nodiscard]] vecRGBA normalized() const { vecRGBA c=*this; return c.normalize(); }
        
        // Geometric rotation of RGB components using an RGB axis (from another vecRGBA), preserves alpha.
        vecRGBA& rotateRGBByAxis(const vecRGBA& axisRGB, double radians) {
            // Use axisRGB.r, axisRGB.g, axisRGB.b as the rotation axis components
            double axisR = axisRGB.r;
            double axisG = axisRGB.g;
            double axisB = axisRGB.b;
            double axisLenSq = axisR*axisR + axisG*axisG + axisB*axisB;

            if (axisLenSq == 0.0) return *this; // No rotation if axis is zero
            double axisLenInv = 1.0 / safe_sqrt(axisLenSq);
            axisR *= axisLenInv; 
            axisG *= axisLenInv; 
            axisB *= axisLenInv;

            double currentR = r, currentG = g, currentB = b; // RGB part to rotate

            double halfAngle = radians * 0.5;
            double s_halfAngle = std::sin(halfAngle);
            double c_halfAngle = std::cos(halfAngle);

            double qx = axisR * s_halfAngle;
            double qy = axisG * s_halfAngle;
            double qz = axisB * s_halfAngle;
            double qw = c_halfAngle;
            
            // Simplified quaternion rotation: v' = v + 2*qw*cross(q_xyz,v) + 2*cross(q_xyz, cross(q_xyz,v))
            // where q_xyz = (qx,qy,qz) and v = (currentR, currentG, currentB)
            vecRGBA q_xyz_vec(qx, qy, qz, 0.0); // Use vecRGBA for cross product helper
            vecRGBA current_rgb_vec(currentR, currentG, currentB, 0.0);
            
            // Manual cross product for RGB part
            auto cross_rgb = [](const vecRGBA& v1, const vecRGBA& v2) -> vecRGBA {
                return {v1.g*v2.b - v1.b*v2.g, 
                        v1.b*v2.r - v1.r*v2.b, 
                        v1.r*v2.g - v1.g*v2.r, 0.0};
            };

            vecRGBA t_vec = cross_rgb(q_xyz_vec, current_rgb_vec) * 2.0;
            vecRGBA rotated_rgb_vec = current_rgb_vec + t_vec * qw + cross_rgb(q_xyz_vec, t_vec);
            
            r = rotated_rgb_vec.r;
            g = rotated_rgb_vec.g;
            b = rotated_rgb_vec.b;
            // Alpha (this->a) remains unchanged
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedRGBByAxis(const vecRGBA& axisRGB, double radians) const {
            vecRGBA copy = *this;
            return copy.rotateRGBByAxis(axisRGB, radians);
        }
        
        // --- Conversions to standard types ---
        std::array<double, 4> toArray() const { return {r, g, b, a}; }
        const double* data() const { return &r; } 
        double* data() { return &r; }

        // --- Operators ---
        [[nodiscard]] vecRGBA operator+(const vecRGBA& o) const { return {r+o.r,g+o.g,b+o.b,a+o.a}; }
        [[nodiscard]] vecRGBA operator-(const vecRGBA& o) const { return {r-o.r,g-o.g,b-o.b,a-o.a}; }
        [[nodiscard]] vecRGBA operator*(const vecRGBA& o) const { return {r*o.r,g*o.g,b*o.b,a*o.a}; } // Modulate
        [[nodiscard]] vecRGBA operator/(const vecRGBA& o) const { return {safe_divide(r,o.r),safe_divide(g,o.g),safe_divide(b,o.b),safe_divide(a,o.a)}; }
        [[nodiscard]] vecRGBA operator-() const { return {-r,-g,-b,-a}; }
        [[nodiscard]] vecRGBA operator+(const double& s) const { return {r+s,g+s,b+s,a+s}; }
        [[nodiscard]] vecRGBA operator-(const double& s) const { return {r-s,g-s,b-s,a-s}; }
        [[nodiscard]] vecRGBA operator*(const double& s) const { return {r*s,g*s,b*s,a*s}; }
        [[nodiscard]] vecRGBA operator/(const double& s) const { return {safe_divide(r,s),safe_divide(g,s),safe_divide(b,s),safe_divide(a,s)}; }

        vecRGBA& operator+=(const vecRGBA& o){r+=o.r;g+=o.g;b+=o.b;a+=o.a;return *this;}
        vecRGBA& operator-=(const vecRGBA& o){r-=o.r;g-=o.g;b-=o.b;a-=o.a;return *this;}
        vecRGBA& operator*=(const vecRGBA& o){r*=o.r;g*=o.g;b*=o.b;a*=o.a;return *this;} 
        vecRGBA& operator/=(const vecRGBA& o){r=safe_divide(r,o.r);g=safe_divide(g,o.g);b=safe_divide(b,o.b);a=safe_divide(a,o.a);return *this;}
        vecRGBA& operator+=(const double& s){r+=s;g+=s;b+=s;a+=s;return *this;}
        vecRGBA& operator-=(const double& s){r-=s;g-=s;b-=s;a-=s;return *this;}
        vecRGBA& operator*=(const double& s){r*=s;g*=s;b*=s;a*=s;return *this;}
        vecRGBA& operator/=(const double& s){r=safe_divide(r,s);g=safe_divide(g,s);b=safe_divide(b,s);a=safe_divide(a,s);return *this;}

        bool operator==(const vecRGBA& o)const{return r==o.r&&g==o.g&&b==o.b&&a==o.a;}
        bool operator!=(const vecRGBA& o)const{return !(*this==o);}
        bool operator<(const vecRGBA& o)const{return magnitudeSquared()<o.magnitudeSquared();}
        bool operator<=(const vecRGBA& o)const{return magnitudeSquared()<=o.magnitudeSquared();}
        bool operator>(const vecRGBA& o)const{return magnitudeSquared()>o.magnitudeSquared();}
        bool operator>=(const vecRGBA& o)const{return magnitudeSquared()>=o.magnitudeSquared();}

        std::string toStr() const { return std::format("vecRGBA[r:{:.3f},g:{:.3f},b:{:.3f},a:{:.3f}]",r,g,b,a); }
    };

    [[nodiscard]]inline vecRGBA operator+(const double& s,const vecRGBA& v){return v+s;}
    [[nodiscard]]inline vecRGBA operator-(const double& s,const vecRGBA& v){return {s-v.r,s-v.g,s-v.b,s-v.a};}
    [[nodiscard]]inline vecRGBA operator*(const double& s,const vecRGBA& v){return v*s;}
    [[nodiscard]]inline vecRGBA operator/(const double& s,const vecRGBA& v){return {safe_divide(s,v.r),safe_divide(s,v.g),safe_divide(s,v.b),safe_divide(s,v.a)};}
    inline std::ostream& operator<<(std::ostream& os,const vecRGBA& v){os<<v.toStr();return os;}

} // End of namespace vecSys
