#pragma once

#include "base.h"

namespace vecSys {

    struct vecRGBA {
        double r = 0.0, g = 0.0, b = 0.0, a = 1.0; // Default to opaque black

        // --- Constructors ---
        vecRGBA() {} // Defaults to 0,0,0,1
        vecRGBA(const double& grayVal, const double& alphaVal = 1.0): r(grayVal), g(grayVal), b(grayVal), a(alphaVal) {}
        vecRGBA(const double& redVal, const double& greenVal, const double& blueVal, const double& alphaVal = 1.0): r(redVal), g(greenVal), b(blueVal), a(alphaVal) {}
        vecRGBA(const std::array<double, 4>& channels): r(channels[0]), g(channels[1]), b(channels[2]), a(channels[3]) {}
        vecRGBA(const std::array<double, 3>& rgbChannels, const double& alphaVal = 1.0): r(rgbChannels[0]), g(rgbChannels[1]), b(rgbChannels[2]), a(alphaVal) {}

        explicit vecRGBA(const std::vector<double>& vals) {
            if (vals.empty()) {
                r = 0.0; g = 0.0; b = 0.0; a = 1.0; // Default to opaque black
            } else if (vals.size() == 1) {
                r = vals[0]; g = vals[0]; b = vals[0]; a = 1.0; // Grayscale opaque
            } else if (vals.size() == 2) {
                r = vals[0]; g = vals[1]; b = 0.0; a = 1.0;    // R, G, B=0, opaque
            } else if (vals.size() == 3) {
                r = vals[0]; g = vals[1]; b = vals[2]; a = 1.0; // RGB, opaque
            } else { // vals.size() >= 4
                r = vals[0]; g = vals[1]; b = vals[2]; a = vals[3]; // RGBA
            }
        }

        vecRGBA(const vecRGBA& other) = default; // Default copy constructor

        // --- Static Factory Methods (all opaque by default) ---
        static constexpr vecRGBA TransparentBlack() {return {0.0,0.0,0.0,0.0};}
        static constexpr vecRGBA Black() {return {0.0,0.0,0.0,1.0};}
        static constexpr vecRGBA DarkGrey() {return {0.25,0.25,0.25,1.0};}
        static constexpr vecRGBA Grey() {return {0.5,0.5,0.5,1.0};}
        static constexpr vecRGBA LightGrey() {return {0.75,0.75,0.75,1.0};}
        static constexpr vecRGBA White() {return {1.0,1.0,1.0,1.0};}

        static constexpr vecRGBA Red() {return {1.0,0.0,0.0,1.0};}
        static constexpr vecRGBA Green() {return {0.0,1.0,0.0,1.0};}
        static constexpr vecRGBA Blue() {return {0.0,0.0,1.0,1.0};}

        static constexpr vecRGBA Yellow() {return {1.0,1.0,0.0,1.0};}
        static constexpr vecRGBA Magenta() {return {1.0,0.0,1.0,1.0};}
        static constexpr vecRGBA Cyan() {return {0.0,1.0,1.0,1.0};}

        static constexpr vecRGBA Orange() {return {1.0, 0.647, 0.0, 1.0};}
        static constexpr vecRGBA Purple() {return {0.5, 0.0, 0.5, 1.0};}
        static constexpr vecRGBA Brown() {return {0.647, 0.165, 0.165, 1.0};}
        static constexpr vecRGBA Pink() {return {1.0, 0.753, 0.796, 1.0};}

        static vecRGBA random(double minVal = 0.0, double maxVal = 1.0) {return vecRGBA().randomize(minVal, maxVal);}
        static vecRGBA randomRGB(double minVal = 0.0, double maxVal = 1.0, double alphaVal = 1.0) {
            vecRGBA col;
            col.randomizeRGB(minVal, maxVal);
            col.a = alphaVal;
            return col;
        }


        // --- Core Color Operations ---
        // Opposite color (inverted RGB in [0,1] range), alpha preserved.
        vecRGBA opposite() const {
            vecRGBA clampedColor = clamped(); // Clamps all channels to [0,1]
            return {1.0 - clampedColor.r, 1.0 - clampedColor.g, 1.0 - clampedColor.b, clampedColor.a};
        }

        // Average color with another color (averages R,G,B,A)
        vecRGBA average(const vecRGBA& other) const { return (*this + other) * 0.5; }

        // Multiplied with another color (component-wise for R,G,B,A)
        vecRGBA multiplyColor(const vecRGBA& other) const { return {r * other.r, g * other.g, b * other.b, a * other.a}; }

        // Luminance (perceptual brightness of RGB components, weights for sRGB). Alpha ignored.
        double luminance() const {
            vecRGBA c = clamped(); // Clamps RGB to [0,1]
            return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
        }

        // Grayscale conversion using luminance for RGB. Alpha preserved.
        vecRGBA grayscale() const {
            double gray = luminance();
            return {gray, gray, gray, a};
        }

        // Euclidean distance in RGB color space. Alpha ignored.
        double colorDistanceRGB(const vecRGBA& other) const {
            double dr = r - other.r;
            double dg = g - other.g;
            double db = b - other.b;
            return safe_sqrt(dr*dr + dg*dg + db*db);
        }

        // --- Color Space Conversions ---
        // RGB to HSV (Hue [0,1], Saturation [0,1], Value [0,1]). Alpha preserved.
        // Assumes RGB components are typically in the range [0,1]; clamps them internally.
        vecRGBA toHSV() const {
            vecRGBA clampedRGB = clamped(); // Ensure input RGB is in [0,1] range
            double R = clampedRGB.r, G = clampedRGB.g, B = clampedRGB.b;

            double maxC = std::max({R, G, B});
            double minC = std::min({R, G, B});
            double delta = maxC - minC;

            double h = 0.0, s = 0.0, v = maxC;

            if (delta > 0.0) {
                if (maxC > 0.0) {
                    s = delta / maxC;
                } else {
                    s = 0.0; h = 0.0;
                    return {h, s, v, a}; // Alpha preserved
                }

                if (std::abs(maxC - R) < default_Tolerance) { // Using tolerance for float comparison
                    h = (G - B) / delta;
                } else if (std::abs(maxC - G) < default_Tolerance) {
                    h = 2.0 + (B - R) / delta;
                } else {
                    h = 4.0 + (R - G) / delta;
                }
                h /= 6.0;
                if (h < 0.0) {
                    h += 1.0;
                }
            } else {
                s = 0.0; h = 0.0;
            }
            return {h, s, v, a}; // Alpha preserved
        }

        // HSV to RGB (expects H, S, V in range [0, 1]). Alpha provided or defaults to 1.0.
        static vecRGBA fromHSV(double h, double s, double v, double alpha = 1.0) {
            h = std::fmod(h, 1.0); if (h < 0.0) h += 1.0;
            s = std::clamp(s, 0.0, 1.0);
            v = std::clamp(v, 0.0, 1.0);

            if (std::abs(s) < default_Tolerance) { // Achromatic (gray)
                return {v, v, v, alpha};
            }

            int i = static_cast<int>(std::floor(h * 6.0));
            double f = (h * 6.0) - static_cast<double>(i);
            double p = v * (1.0 - s);
            double q = v * (1.0 - s * f);
            double t = v * (1.0 - s * (1.0 - f));

            switch (i % 6) {
                case 0: return {v, t, p, alpha};
                case 1: return {q, v, p, alpha};
                case 2: return {p, v, t, alpha};
                case 3: return {p, q, v, alpha};
                case 4: return {t, p, v, alpha};
                case 5: return {v, p, q, alpha};
                default: return {0.0, 0.0, 0.0, alpha}; // Should not be reached
            }
        }

        // RGB to HSL (Hue [0,1], Saturation [0,1], Lightness [0,1]). Alpha preserved.
        vecRGBA toHSL() const {
            vecRGBA clampedRGB = clamped();
            double R = clampedRGB.r, G = clampedRGB.g, B = clampedRGB.b;

            double maxC = std::max({R, G, B});
            double minC = std::min({R, G, B});
            double delta = maxC - minC;

            double h = 0.0, s = 0.0, l = (maxC + minC) * 0.5;

            if (delta > default_Tolerance) {
                if (l == 0.0 || l == 1.0) {
                     s = 0.0;
                } else {
                    s = (l < 0.5) ? (delta / (maxC + minC)) : (delta / (2.0 - maxC - minC));
                }
                
                if (std::abs(maxC - R) < default_Tolerance) {
                    h = (G - B) / delta + (G < B ? 6.0 : 0.0);
                } else if (std::abs(maxC - G) < default_Tolerance) {
                    h = (B - R) / delta + 2.0;
                } else {
                    h = (R - G) / delta + 4.0;
                }
                h /= 6.0;
            }
            return {h, s, l, a}; // Alpha preserved
        }

        // HSL to RGB (expects H, S, L in range [0, 1]). Alpha provided or defaults to 1.0.
        static vecRGBA fromHSL(double h, double s, double l, double alpha = 1.0) {
            h = std::fmod(h, 1.0); if (h < 0.0) h += 1.0;
            s = std::clamp(s, 0.0, 1.0);
            l = std::clamp(l, 0.0, 1.0);

            if (std::abs(s) < default_Tolerance) { // Achromatic
                return {l, l, l, alpha};
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
                double R = hueToChannel(p_val, q_val, h + 1.0 / 3.0);
                double G = hueToChannel(p_val, q_val, h);
                double B = hueToChannel(p_val, q_val, h - 1.0 / 3.0);
                return {R, G, B, alpha};
            }
        }

        // --- Integer and Hex String Conversions ---
        // Converts to 8-bit RGBA components [0-255]
        [[nodiscard]] std::array<uint8_t, 4> toRGBA8() const {
            vecRGBA c = clamped(); // Ensure color is in [0,1] before conversion
            return {
                static_cast<uint8_t>(std::round(c.r * 255.0)),
                static_cast<uint8_t>(std::round(c.g * 255.0)),
                static_cast<uint8_t>(std::round(c.b * 255.0)),
                static_cast<uint8_t>(std::round(c.a * 255.0))
            };
        }

        // Creates from 8-bit RGBA components [0-255]
        static vecRGBA fromRGBA8(uint8_t r_byte, uint8_t g_byte, uint8_t b_byte, uint8_t a_byte) {
            return {
                static_cast<double>(r_byte) / 255.0,
                static_cast<double>(g_byte) / 255.0,
                static_cast<double>(b_byte) / 255.0,
                static_cast<double>(a_byte) / 255.0
            };
        }
        static vecRGBA fromRGBA8(const std::array<uint8_t, 4>& rgba_bytes) {
            return fromRGBA8(rgba_bytes[0], rgba_bytes[1], rgba_bytes[2], rgba_bytes[3]);
        }
         static vecRGBA fromRGB8(uint8_t r_byte, uint8_t g_byte, uint8_t b_byte) { // For convenience if alpha is 255
            return fromRGBA8(r_byte, g_byte, b_byte, 255);
        }


        // Converts to HEX string (e.g., "#RRGGBBAA" or "#RRGGBB")
        [[nodiscard]] std::string toHexString(bool includeHash = true, bool includeAlphaIfNonOpaque = true) const {
            std::array<uint8_t, 4> rgba8 = toRGBA8();
            std::ostringstream oss;
            if (includeHash) {
                oss << "#";
            }
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[0]);
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[1]);
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[2]);
            if (includeAlphaIfNonOpaque && rgba8[3] != 255) {
                 oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[3]);
            } else if (!includeAlphaIfNonOpaque && rgba8[3] == 255 && toHexString(false,true).length() == 8) { // special case for always showing alpha if it was shown before
                 // this logic is a bit complex, user might want simpler always include alpha flag
            }
            // Simplified: always include alpha if not 255, or if explicitly requested for opaque
            // For this version: include alpha if includeAlphaIfNonOpaque is true AND (alpha is not 255 OR a flag forces it)
            // Let's make it simpler: add a bool forceIncludeAlphaHex
            // For now, using the provided logic:
            if (includeAlphaIfNonOpaque) { // if this flag is true, means we *might* include alpha
                if (rgba8[3] != 255) { // if alpha is not opaque, always include it
                    oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rgba8[3]);
                }
                // if alpha is opaque (255), it's omitted by this logic if includeAlphaIfNonOpaque is true
            }
            // A better toHexString:
            // [[nodiscard]] std::string toHexString(bool includeHash = true, bool forceAlpha = false) const {
            // ...
            // if (forceAlpha || rgba8[3] != 255) { oss << ... rgba8[3] ...; }
            // return oss.str();
            // }
            // For now, sticking to a simpler interpretation for the current toHexString:
            // if includeAlphaIfNonOpaque is true, it means "include alpha if it's meaningful (i.e., not 255)"
            // if false, never include alpha hex.
            // Let's refine:
            // toHexString(bool includeHash = true, bool hexIncludesAlpha = false)
            // if hexIncludesAlpha is true, it's RRGGBBAA. Otherwise RRGGBB.
            // For this implementation, let's use the boolean to decide if AA is appended.
            // std::string toHexString(bool includeHash = true, bool outputAlphaChannel = true) const
            // if (outputAlphaChannel) add AA part.
            // The original name was includeAlphaIfNonOpaque. Let's stick to that for now.
            // If true, and alpha is not 255, it's added. If false, it's never added.
            return oss.str();
        }
        
        // Simpler toHexString
        [[nodiscard]] std::string toHex(bool includeHash = true, bool includeAlphaHex = false) const {
            std::array<uint8_t, 4> rgba8 = toRGBA8();
            std::ostringstream oss;
            if (includeHash) oss << "#";
            oss << std::hex << std::setfill('0')
                << std::setw(2) << static_cast<int>(rgba8[0])
                << std::setw(2) << static_cast<int>(rgba8[1])
                << std::setw(2) << static_cast<int>(rgba8[2]);
            if (includeAlphaHex) {
                oss << std::setw(2) << static_cast<int>(rgba8[3]);
            }
            return oss.str();
        }


        // Creates from HEX string (e.g., "#RRGGBB", "RRGGBB", "#RRGGBBAA", "RRGGBBAA")
        static vecRGBA fromHexString(std::string hexStr) {
            if (hexStr.empty()) {
                throw std::invalid_argument("Hex string cannot be empty.");
            }
            if (hexStr[0] == '#') {
                hexStr.erase(0, 1);
            }

            if (hexStr.length() != 6 && hexStr.length() != 8) {
                throw std::invalid_argument("Hex string must be 6 or 8 characters long (excluding optional '#').");
            }

            try {
                unsigned long r_val = std::stoul(hexStr.substr(0, 2), nullptr, 16);
                unsigned long g_val = std::stoul(hexStr.substr(2, 2), nullptr, 16);
                unsigned long b_val = std::stoul(hexStr.substr(4, 2), nullptr, 16);
                unsigned long a_val = 255; // Default to opaque
                if (hexStr.length() == 8) {
                    a_val = std::stoul(hexStr.substr(6, 2), nullptr, 16);
                }
                return fromRGBA8(static_cast<uint8_t>(r_val), static_cast<uint8_t>(g_val), static_cast<uint8_t>(b_val), static_cast<uint8_t>(a_val));
            } catch (const std::invalid_argument& ia) {
                throw std::invalid_argument("Invalid character in hex string: " + hexStr + ". " + ia.what());
            } catch (const std::out_of_range& oor) {
                 throw std::out_of_range("Hex value out of range: " + hexStr + ". " + oor.what());
            }
        }

        // --- Blending Modes ---
        // These methods clamp their inputs internally to ensure results are within typical [0,1] color range.
        vecRGBA blendMultiply(const vecRGBA& other) const {
            vecRGBA baseClamped = clamped();
            vecRGBA otherClamped = other.clamped();
            return {baseClamped.r * otherClamped.r,
                    baseClamped.g * otherClamped.g,
                    baseClamped.b * otherClamped.b,
                    std::clamp(baseClamped.a * otherClamped.a, 0.0, 1.0)}; // Alpha multiply
        }

        vecRGBA blendScreen(const vecRGBA& other) const {
            vecRGBA baseClamped = clamped();
            vecRGBA otherClamped = other.clamped();
            // RGB: 1 - (1 - Base) * (1 - Blend)
            // Alpha: A_base + A_blend - A_base * A_blend
            return {1.0 - (1.0 - baseClamped.r) * (1.0 - otherClamped.r),
                    1.0 - (1.0 - baseClamped.g) * (1.0 - otherClamped.g),
                    1.0 - (1.0 - baseClamped.b) * (1.0 - otherClamped.b),
                    std::clamp(baseClamped.a + otherClamped.a - baseClamped.a * otherClamped.a, 0.0, 1.0)};
        }

        vecRGBA blendOverlay(const vecRGBA& other) const {
            vecRGBA base = clamped();
            vecRGBA blend = other.clamped();
            auto overlayComponent = [](double b_comp, double o_comp) -> double {
                return (b_comp < 0.5) ? (2.0 * b_comp * o_comp) : (1.0 - 2.0 * (1.0 - b_comp) * (1.0 - o_comp));
            };
            // For alpha in overlay, often the base alpha is used, or a more complex compositing rule.
            // Here, let's use the base alpha, or average for a softer effect.
            // Using average alpha for a general overlay effect:
            double blendedAlpha = std::clamp((base.a + blend.a) * 0.5, 0.0, 1.0);
            return {overlayComponent(base.r, blend.r),
                    overlayComponent(base.g, blend.g),
                    overlayComponent(base.b, blend.b),
                    blendedAlpha};
        }

        vecRGBA blendAdditive(const vecRGBA& other) const {
            vecRGBA baseClamped = clamped();
            vecRGBA otherClamped = other.clamped();
            return {(baseClamped + otherClamped).clamped()}; // Clamps all components R,G,B,A after sum
        }

        vecRGBA blendSubtract(const vecRGBA& other) const {
            vecRGBA baseClamped = clamped();
            vecRGBA otherClamped = other.clamped();
            return {(baseClamped - otherClamped).clamped()}; // Clamps all components R,G,B,A after difference
        }

        // --- Alpha Compositing ---
        // Premultiplies RGB components by alpha.
        vecRGBA& premultiplyAlpha() {
            r *= a; g *= a; b *= a;
            return *this;
        }
        [[nodiscard]] vecRGBA premultipliedAlpha() const { vecRGBA c = *this; return c.premultiplyAlpha(); }

        // Unpremultiplies RGB components by alpha. Handles alpha = 0.
        vecRGBA& unpremultiplyAlpha() {
            if (std::abs(a) > default_Tolerance) { // Check a is not zero
                r = safe_divide(r, a);
                g = safe_divide(g, a);
                b = safe_divide(b, a);
            } else { // If alpha is zero, color components should also be zero
                r = 0.0; g = 0.0; b = 0.0;
            }
            return *this;
        }
        [[nodiscard]] vecRGBA unpremultipliedAlpha() const { vecRGBA c = *this; return c.unpremultiplyAlpha(); }

        // Composites this color (foreground) OVER a background color. Assumes non-premultiplied inputs.
        // Result is non-premultiplied.
        vecRGBA compositeOver(const vecRGBA& background) const {
            // this = Foreground (F), background = Background (B)
            // Output Alpha: Ao = Af + Ab * (1 - Af)
            // Output Color: Co = (Cf * Af + Cb * Ab * (1 - Af)) / Ao
            // If Ao is 0, Co is (0,0,0)
            
            vecRGBA F = *this; // Foreground
            vecRGBA B = background;

            double out_a = F.a + B.a * (1.0 - F.a);
            out_a = std::clamp(out_a, 0.0, 1.0); // Ensure alpha is valid

            double out_r = 0.0, out_g = 0.0, out_b = 0.0;

            if (std::abs(out_a) > default_Tolerance) { // If output alpha is not zero
                out_r = (F.r * F.a + B.r * B.a * (1.0 - F.a)) / out_a;
                out_g = (F.g * F.a + B.g * B.a * (1.0 - F.a)) / out_a;
                out_b = (F.b * F.a + B.b * B.a * (1.0 - F.a)) / out_a;
            }
            // Clamp final color components as they might be slightly outside [0,1] due to precision
            return {std::clamp(out_r, 0.0, 1.0),
                    std::clamp(out_g, 0.0, 1.0),
                    std::clamp(out_b, 0.0, 1.0),
                    out_a};
        }


        // --- Color Transformations ---
        // hueShiftAmount_0_to_1 is normalized (0 to 1). Alpha preserved.
        vecRGBA adjustHue(double hueShiftAmount_0_to_1) const {
            vecRGBA hsv = toHSV(); // Alpha is already in hsv.a
            hsv.r = std::fmod(hsv.r + hueShiftAmount_0_to_1, 1.0);
            if (hsv.r < 0.0) hsv.r += 1.0;
            return vecRGBA::fromHSV(hsv.r, hsv.g, hsv.b, hsv.a);
        }

        // saturationDelta typically from -1.0 to 1.0. Alpha preserved.
        vecRGBA adjustSaturation(double saturationDelta) const {
            vecRGBA hsv = toHSV();
            hsv.g = std::clamp(hsv.g + saturationDelta, 0.0, 1.0);
            return vecRGBA::fromHSV(hsv.r, hsv.g, hsv.b, hsv.a);
        }

        // brightnessDelta typically from -1.0 to 1.0. Adjusts Value in HSV. Alpha preserved.
        vecRGBA adjustBrightness(double brightnessDelta) const {
            vecRGBA hsv = toHSV();
            hsv.b = std::clamp(hsv.b + brightnessDelta, 0.0, 1.0);
            return vecRGBA::fromHSV(hsv.r, hsv.g, hsv.b, hsv.a);
        }

        // Adjusts contrast of RGB. `contrastFactor`: 0=mid-grey, 1=no change. Alpha preserved.
        vecRGBA adjustContrast(double contrastFactor) const {
            double factor = std::max(0.0, contrastFactor);
            vecRGBA clampedBase = clamped(); // Clamps all channels, including alpha, to [0,1]
            return {
                std::clamp(0.5 + factor * (clampedBase.r - 0.5), 0.0, 1.0),
                std::clamp(0.5 + factor * (clampedBase.g - 0.5), 0.0, 1.0),
                std::clamp(0.5 + factor * (clampedBase.b - 0.5), 0.0, 1.0),
                clampedBase.a // Preserve original (clamped) alpha
            };
        }

        // Adjusts alpha component.
        vecRGBA& adjustAlpha(double alphaDelta) {
            a = std::clamp(a + alphaDelta, 0.0, 1.0);
            return *this;
        }
        [[nodiscard]] vecRGBA adjustedAlpha(double alphaDelta) const { vecRGBA c = *this; return c.adjustAlpha(alphaDelta); }

        // Sets alpha component.
        vecRGBA& setAlpha(double newAlpha) {
            a = std::clamp(newAlpha, 0.0, 1.0);
            return *this;
        }
        [[nodiscard]] vecRGBA withAlpha(double newAlpha) const { vecRGBA c = *this; return c.setAlpha(newAlpha); }


        // --- "Vector-like" Operations (Treating RGB as coordinates, Alpha generally preserved) ---
        // Angle related methods (interpret R,G,B as X,Y,Z). Alpha ignored.
        double angleR_GBplane() const { return std::atan2(g, b); } // Angle in the G-B plane
        double angleG_BRplane() const { return std::atan2(b, r); } // Angle in the B-R plane
        double angleB_RGplane() const { return std::atan2(g, r); } // Angle in the R-G plane

        // Angle between this vector's RGB components and another's RGB components.
        double angleToRGB(const vecRGBA& other) const {
            // Calculate dot product of RGB components
            double dotProd = r * other.r + g * other.g + b * other.b;
            // Calculate magnitudes of RGB components
            double magThisSq = r*r + g*g + b*b;
            double magOtherSq = other.r*other.r + other.g*other.g + other.b*other.b;

            if (magThisSq == 0.0 || magOtherSq == 0.0) return 0.0; // Angle with zero vector

            double val = dotProd / (safe_sqrt(magThisSq) * safe_sqrt(magOtherSq));
            val = std::clamp(val, -1.0, 1.0); // Clamp for acos safety
            return std::acos(val);
        }
        
        // Returns a vecRGBA where r,g,b are the normalized angle differences in respective planes. Alpha is 0.
        [[nodiscard]] vecRGBA getProjectedAngleDiffsRGB(const vecRGBA& other) const {
            return {
                wrapAngle(other.angleR_GBplane() - angleR_GBplane()),
                wrapAngle(other.angleG_BRplane() - angleG_BRplane()),
                wrapAngle(other.angleB_RGplane() - angleB_RGplane()),
                0.0 // Alpha for this "difference vector" is set to 0, or could be this->a
            };
        }


        // --- Rounding methods (applies to R,G,B,A) ---
        vecRGBA& round() { r = std::round(r); g = std::round(g); b = std::round(b); a = std::round(a); return *this; }
        [[nodiscard]] vecRGBA rounded() const { return {std::round(r), std::round(g), std::round(b), std::round(a)}; }

        // --- Clamping methods ---
        // Clamps all channels (R,G,B,A) to [minVal, maxVal]. Default [0,1].
        vecRGBA& clamp(const double& minVal = 0.0, const double& maxVal = 1.0) {
            r = std::clamp(r, minVal, maxVal);
            g = std::clamp(g, minVal, maxVal);
            b = std::clamp(b, minVal, maxVal);
            a = std::clamp(a, minVal, maxVal);
            return *this;
        }
        [[nodiscard]] vecRGBA clamped(const double& minVal = 0.0, const double& maxVal = 1.0) const {
            return {std::clamp(r, minVal, maxVal), std::clamp(g, minVal, maxVal),
                    std::clamp(b, minVal, maxVal), std::clamp(a, minVal, maxVal)};
        }

        // Component-wise vector clamp for all channels (R,G,B,A).
        vecRGBA& clamp(const vecRGBA& minVec, const vecRGBA& maxVec) {
            r = std::clamp(r, minVec.r, maxVec.r);
            g = std::clamp(g, minVec.g, maxVec.g);
            b = std::clamp(b, minVec.b, maxVec.b);
            a = std::clamp(a, minVec.a, maxVec.a);
            return *this;
        }
        [[nodiscard]] vecRGBA clamped(const vecRGBA& minVec, const vecRGBA& maxVec) const {
            return {std::clamp(r, minVec.r, maxVec.r),
                    std::clamp(g, minVec.g, maxVec.g),
                    std::clamp(b, minVec.b, maxVec.b),
                    std::clamp(a, minVec.a, maxVec.a)};
        }

        // Clamps the magnitude of the RGB components. Alpha preserved.
        vecRGBA& clampMagnitudeRGB(double minLen, double maxLen) {
            double currentLenSq = r*r + g*g + b*b;
            if (currentLenSq == 0.0 && minLen > 0.0) { // Cannot give direction to zero vector if minLen > 0
                 // Optionally set to minLen along an axis, e.g. r = minLen, g=0, b=0
                 // For now, if it's zero, it stays zero unless minLen is positive and non-zero.
                return *this;
            }
             if (currentLenSq == 0.0) return *this; // Stays zero if minLen is also zero or less

            double currentLen = safe_sqrt(currentLenSq);
            double scaleFactor = 1.0;

            if (currentLen < minLen && minLen > 0.0) {
                scaleFactor = safe_divide(minLen, currentLen);
            } else if (currentLen > maxLen && maxLen >= 0.0) {
                scaleFactor = safe_divide(maxLen, currentLen);
            }

            if (std::abs(scaleFactor - 1.0) > default_Tolerance) {
                 if (scaleFactor == 0.0 && maxLen == 0.0) { // Scaling to zero magnitude
                     r = 0.0; g = 0.0; b = 0.0;
                 } else if (scaleFactor != 0.0) {
                     r *= scaleFactor; g *= scaleFactor; b *= scaleFactor;
                 }
            }
            // Alpha (a) is preserved
            return *this;
        }
        [[nodiscard]] vecRGBA clampedMagnitudeRGB(double minLen, double maxLen) const {
            vecRGBA copy = *this;
            return copy.clampMagnitudeRGB(minLen, maxLen);
        }

        // --- Cross Product (Geometric interpretation of RGB components) ---
        // Resulting alpha is from this vector.
        [[nodiscard]] vecRGBA crossRGB(const vecRGBA& other) const {
            return {g * other.b - b * other.g,
                    b * other.r - r * other.b,
                    r * other.g - g * other.r,
                    a}; // Alpha from this vector
        }

        // --- Dot Product (Geometric interpretation of RGB components) ---
        double dotRGB(const vecRGBA& other) const { return r * other.r + g * other.g + b * other.b; }

        // --- Finite/Infinite/NaN Checks (for all components R,G,B,A) ---
        bool isFinite() const { return std::isfinite(r) && std::isfinite(g) && std::isfinite(b) && std::isfinite(a); }
        bool isInfinite() const { return std::isinf(r) || std::isinf(g) || std::isinf(b) || std::isinf(a); }
        bool isNaN() const { return std::isnan(r) || std::isnan(g) || std::isnan(b) || std::isnan(a); }

        // Checks if RGB components are black (all 0.0). Alpha can be anything.
        bool isBlackRGB() const { return r == 0.0 && g == 0.0 && b == 0.0; }
        // Checks if R,G,B,A are all zero.
        bool isZeroRGBA() const { return r == 0.0 && g == 0.0 && b == 0.0 && a == 0.0; }
        // Checks if color is fully transparent.
        bool isTransparent() const { return a == 0.0; }


        // --- Magnitude methods (Geometric interpretation of RGB components) ---
        double magnitudeRGB() const {
            if (std::isinf(r) || std::isinf(g) || std::isinf(b)) return infinity;
            return std::hypot(r, g, b);
        }
        double magnitudeSquaredRGB() const {
            if (std::isinf(r) || std::isinf(g) || std::isinf(b)) return infinity;
            return r*r + g*g + b*b;
        }

        // --- Linear Interpolation (for R,G,B,A) ---
        // Amount is clamped to [0,1].
        vecRGBA& lerp(const vecRGBA& target, const double& amount) {
            double t = std::clamp(amount, 0.0, 1.0);
            r = std::lerp(r, target.r, t);
            g = std::lerp(g, target.g, t);
            b = std::lerp(b, target.b, t);
            a = std::lerp(a, target.a, t);
            return *this;
        }
        [[nodiscard]] vecRGBA lerped(const vecRGBA& target, const double& amount) const {
            vecRGBA copy = *this;
            return copy.lerp(target, amount);
        }

        // --- Component-wise Max/Min (for R,G,B,A) ---
        vecRGBA& setMax(const vecRGBA& other) { r=std::max(r,other.r); g=std::max(g,other.g); b=std::max(b,other.b); a=std::max(a,other.a); return *this; }
        [[nodiscard]] vecRGBA max(const vecRGBA& other) const { return {std::max(r,other.r), std::max(g,other.g), std::max(b,other.b), std::max(a,other.a)}; }
        vecRGBA& setMin(const vecRGBA& other) { r=std::min(r,other.r); g=std::min(g,other.g); b=std::min(b,other.b); a=std::min(a,other.a); return *this; }
        [[nodiscard]] vecRGBA min(const vecRGBA& other) const { return {std::min(r,other.r), std::min(g,other.g), std::min(b,other.b), std::min(a,other.a)}; }

        // --- Move Towards ---
        // Moves R,G,B,A components towards target. Distance calculated on RGB.
        vecRGBA& moveTowards(const vecRGBA& target, double maxDistanceRGB) {
            if (maxDistanceRGB <= 0.0) return *this;
            
            double dr = target.r - r;
            double dg = target.g - g;
            double db = target.b - b;
            // Alpha difference for lerping alpha component consistently
            double da = target.a - a;


            double distSqRGB = dr*dr + dg*dg + db*db;

            if (distSqRGB == 0.0 || distSqRGB <= maxDistanceRGB * maxDistanceRGB) {
                *this = target; // Already at or within range for RGB, so snap all components
            } else {
                double distRGB = safe_sqrt(distSqRGB); // distRGB will be > 0
                double moveFactor = maxDistanceRGB / distRGB; // Ratio of movement for RGB
                r += dr * moveFactor;
                g += dg * moveFactor;
                b += db * moveFactor;
                a += da * moveFactor; // Move alpha proportionally to RGB movement
                // Ensure alpha remains clamped, as proportional move might exceed [0,1] if target.a is far
                a = std::clamp(a, 0.0, 1.0); 
            }
            return *this;
        }
        [[nodiscard]] vecRGBA movedTowards(const vecRGBA& target, double maxDistanceRGB) const {
            vecRGBA copy = *this;
            return copy.moveTowards(target, maxDistanceRGB);
        }

        // --- Normalization (Geometric interpretation - to unit vector in RGB space). Alpha preserved. ---
        vecRGBA& normalizeRGB() {
            if (isBlackRGB() && !(std::isinf(r) || std::isinf(g) || std::isinf(b))) return *this; // Cannot normalize zero RGB vector unless it's from infinities

            if (std::isinf(r) || std::isinf(g) || std::isinf(b)) {
                double sign_r = (std::isinf(r) ? std::copysign(1.0, r) : (std::isnan(r) ? 0.0 : r) );
                double sign_g = (std::isinf(g) ? std::copysign(1.0, g) : (std::isnan(g) ? 0.0 : g) );
                double sign_b = (std::isinf(b) ? std::copysign(1.0, b) : (std::isnan(b) ? 0.0 : b) );
                // Normalize the signs vector
                double tempLen = std::hypot(sign_r, sign_g, sign_b);
                if (tempLen > default_Tolerance) {
                    r = sign_r / tempLen; g = sign_g / tempLen; b = sign_b / tempLen;
                } else { // All components were 0 after sign check or became NaN resulting in 0
                    r = 1.0; g = 0.0; b = 0.0; // Default to unit X
                }
                // Alpha (a) is preserved
                return *this;
            }

            double len = magnitudeRGB();
            if (len == 0.0) return *this; // Should be caught by isBlackRGB(), safeguard
            r /= len; g /= len; b /= len;
            // Alpha (a) is preserved
            return *this;
        }
        [[nodiscard]] vecRGBA normalizedRGB() const {
            vecRGBA copy = *this;
            return copy.normalizeRGB();
        }

        // --- Geometric Rotation Operations (Treating RGB as coordinates). Alpha preserved. ---
        // Axis alpha is ignored. Result alpha is from this vector.
        vecRGBA& rotateByAxisAngleRGB(const vecRGBA& axisRGB, double radians) {
            // Normalize axis's RGB components
            double ax = axisRGB.r, ay = axisRGB.g, az = axisRGB.b;
            double axisMag = std::hypot(ax, ay, az);
            if (axisMag == 0.0) return *this; // Cannot rotate around zero axis
            ax /= axisMag; ay /= axisMag; az /= axisMag;

            double halfAngle = radians * 0.5;
            double s_halfAngle = std::sin(halfAngle);
            double c_halfAngle = std::cos(halfAngle);

            // Quaternion components for rotation
            double qx = ax * s_halfAngle;
            double qy = ay * s_halfAngle;
            double qz = az * s_halfAngle;
            double qw = c_halfAngle;

            // Vector components to rotate (this->r, this->g, this->b)
            double vx = r, vy = g, vz = b;

            // cross1 = cross(q_xyz, v_xyz)
            double t_x = 2.0 * (qy * vz - qz * vy);
            double t_y = 2.0 * (qz * vx - qx * vz);
            double t_z = 2.0 * (qx * vy - qy * vx);

            // cross2 = cross(q_xyz, t)
            double cross2_x = qy * t_z - qz * t_y;
            double cross2_y = qz * t_x - qx * t_z;
            double cross2_z = qx * t_y - qy * t_x;
            
            r = vx + qw * t_x + cross2_x;
            g = vy + qw * t_y + cross2_y;
            b = vz + qw * t_z + cross2_z;
            // Alpha (a) is preserved
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedByAxisAngleRGB(const vecRGBA& axisRGB, double radians) const {
            vecRGBA copy = *this;
            return copy.rotateByAxisAngleRGB(axisRGB, radians);
        }

        // Axial rotations on RGB components. Alpha preserved.
        vecRGBA& rotateR_aroundRGB(const double& radians) { // Rotates G,B around R-axis
            double current_g = g, current_b = b;
            double cosA = std::cos(radians);
            double sinA = std::sin(radians);
            g = current_g * cosA - current_b * sinA;
            b = current_g * sinA + current_b * cosA;
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedR_aroundRGB(const double& radians) const { vecRGBA copy = *this; return copy.rotateR_aroundRGB(radians); }

        vecRGBA& rotateG_aroundRGB(const double& radians) { // Rotates R,B around G-axis
            double current_r = r, current_b = b;
            double cosA = std::cos(radians);
            double sinA = std::sin(radians);
            r = current_r * cosA - current_b * sinA;
            b = current_r * sinA + current_b * cosA;
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedG_aroundRGB(const double& radians) const { vecRGBA copy = *this; return copy.rotateG_aroundRGB(radians); }

        vecRGBA& rotateB_aroundRGB(const double& radians) { // Rotates R,G around B-axis
            double current_r = r, current_g = g;
            double cosA = std::cos(radians);
            double sinA = std::sin(radians);
            r = current_r * cosA - current_g * sinA;
            g = current_r * sinA + current_g * cosA;
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedB_aroundRGB(const double& radians) const { vecRGBA copy = *this; return copy.rotateB_aroundRGB(radians); }

        // Lerped axial rotations on RGB. Alpha preserved.
        vecRGBA& rotateLerpR_aroundRGB(const double& angleDelta, const double& rate) {
            double currentAngleInBG = std::atan2(g, b);
            double targetAngleInBG = currentAngleInBG + angleDelta;
            double diff = wrapAngle(targetAngleInBG - currentAngleInBG);
            double lerped_offset = std::lerp(0.0, diff, std::clamp(rate,0.0,1.0));
            double newAngleInBG = currentAngleInBG + lerped_offset;
            double magInBG = std::hypot(g, b);
            if (magInBG > default_Tolerance) {
                g = magInBG * std::sin(newAngleInBG);
                b = magInBG * std::cos(newAngleInBG);
            } // else g,b are zero, no change
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedLerpR_aroundRGB(const double& angleDelta, const double& rate) const { vecRGBA copy = *this; return copy.rotateLerpR_aroundRGB(angleDelta, rate); }
        // Similar for G and B lerped rotations
        vecRGBA& rotateLerpG_aroundRGB(const double& angleDelta, const double& rate) { // Rotates R,B plane around G
            double currentAngleInRB = std::atan2(b, r); // Note: atan2(y,x) -> atan2(b,r) for rotation in R-B plane
            double targetAngleInRB = currentAngleInRB + angleDelta;
            double diff = wrapAngle(targetAngleInRB - currentAngleInRB);
            double lerped_offset = std::lerp(0.0, diff, std::clamp(rate,0.0,1.0));
            double newAngleInRB = currentAngleInRB + lerped_offset;
            double magInRB = std::hypot(r, b);
            if (magInRB > default_Tolerance) {
                b = magInRB * std::sin(newAngleInRB); // b is 'y'
                r = magInRB * std::cos(newAngleInRB); // r is 'x'
            }
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedLerpG_aroundRGB(const double& angleDelta, const double& rate) const { vecRGBA copy = *this; return copy.rotateLerpG_aroundRGB(angleDelta, rate); }

        vecRGBA& rotateLerpB_aroundRGB(const double& angleDelta, const double& rate) { // Rotates R,G plane around B
            double currentAngleInRG = std::atan2(g, r); // atan2(g,r) for rotation in R-G plane
            double targetAngleInRG = currentAngleInRG + angleDelta;
            double diff = wrapAngle(targetAngleInRG - currentAngleInRG);
            double lerped_offset = std::lerp(0.0, diff, std::clamp(rate,0.0,1.0));
            double newAngleInRG = currentAngleInRG + lerped_offset;
            double magInRG = std::hypot(r, g);
            if (magInRG > default_Tolerance) {
                g = magInRG * std::sin(newAngleInRG); // g is 'y'
                r = magInRG * std::cos(newAngleInRG); // r is 'x'
            }
            return *this;
        }
        [[nodiscard]] vecRGBA rotatedLerpB_aroundRGB(const double& angleDelta, const double& rate) const { vecRGBA copy = *this; return copy.rotateLerpB_aroundRGB(angleDelta, rate); }


        // Orbits RGB components around point's RGB, using axis's RGB. Alpha preserved.
        vecRGBA& orbitRGB(const vecRGBA& pointRGB, const vecRGBA& axisRGB, double radians) {
            // Translate so pointRGB is the origin
            r -= pointRGB.r; g -= pointRGB.g; b -= pointRGB.b;
            // Rotate around the axis (which is now relative to the new origin)
            rotateByAxisAngleRGB(axisRGB, radians);
            // Translate back
            r += pointRGB.r; g += pointRGB.g; b += pointRGB.b;
            // Alpha (a) is preserved
            return *this;
        }
        [[nodiscard]] vecRGBA orbitedRGB(const vecRGBA& pointRGB, const vecRGBA& axisRGB, double radians) const {
            vecRGBA copy = *this;
            return copy.orbitRGB(pointRGB, axisRGB, radians);
        }

        // --- Randomization ---
        // Randomizes R,G,B,A components.
        vecRGBA& randomize(double minVal = 0.0, double maxVal = 1.0) {
            if (minVal > maxVal) std::swap(minVal, maxVal);
            static std::mt19937_64 rng{std::random_device{}()}; // Mersenne Twister 64-bit
            std::uniform_real_distribution<double> distribution(minVal, maxVal);
            r = distribution(rng); g = distribution(rng); b = distribution(rng); a = distribution(rng);
            return *this;
        }
        [[nodiscard]] vecRGBA randomized(double minVal = 0.0, double maxVal = 1.0) const {
            vecRGBA copy; return copy.randomize(minVal, maxVal);
        }
        // Randomizes only RGB components, alpha is set.
        vecRGBA& randomizeRGB(double minVal = 0.0, double maxVal = 1.0) {
             if (minVal > maxVal) std::swap(minVal, maxVal);
            static std::mt19937_64 rng_rgb{std::random_device{}()};
            std::uniform_real_distribution<double> distribution(minVal, maxVal);
            r = distribution(rng_rgb); g = distribution(rng_rgb); b = distribution(rng_rgb);
            // Alpha (a) is preserved
            return *this;
        }


        // --- Scaling ---
        // Scales RGB components by scalar. Alpha preserved. (Vector3-like behavior)
        vecRGBA& scaleRGB(const double& scalar) { r *= scalar; g *= scalar; b *= scalar; return *this; }
        [[nodiscard]] vecRGBA scaledRGB(const double& scalar) const { return {r * scalar, g * scalar, b * scalar, a}; }

        // --- Conversions to standard types ---
        std::array<double, 4> toArray() const { return {r, g, b, a}; }
        const double* data() const { return &r; }
        double* data() { return &r; }

        // --- Static Utility Methods (Geometric interpretation of RGB) ---
        // Ortho-normalizes the RGB components of u and v. Alphas are preserved.
        static void orthoNormalizeRGB(vecRGBA& u, vecRGBA& v) {
            double u_alpha = u.a; // Preserve original alphas
            double v_alpha = v.a;

            // Temp variables for u's RGB
            double ur = u.r, ug = u.g, ub = u.b;
            // Temp variables for v's RGB (original values for cross product)
            double vr_orig = v.r, vg_orig = v.g, vb_orig = v.b;

            // Normalize u_rgb
            double mag_u = std::hypot(ur, ug, ub);
            if (mag_u > default_Tolerance) {
                ur /= mag_u; ug /= mag_u; ub /= mag_u;
            } else { // u_rgb is zero vector, set to a default axis
                ur = 1.0; ug = 0.0; ub = 0.0;
            }

            // temp_rgb = cross_product(u_rgb, v_rgb_orig)
            double c1r = ug * vb_orig - ub * vg_orig;
            double c1g = ub * vr_orig - ur * vb_orig;
            double c1b = ur * vg_orig - ug * vr_orig;

            // Normalize temp_rgb (c1)
            double mag_c1 = std::hypot(c1r, c1g, c1b);
            if (mag_c1 > default_Tolerance) {
                c1r /= mag_c1; c1g /= mag_c1; c1b /= mag_c1;
            } else { // u_rgb and v_rgb_orig were parallel. Pick an arbitrary orthogonal for c1.
                // Robustly find a vector orthogonal to (ur, ug, ub)
                if (std::abs(ur) > 0.9) { // If u is mostly along X
                    c1r = 0.0; c1g = -ub; c1b = ug; // Cross with (0,Y,Z) like vector, e.g. (0, -u.z, u.y)
                } else { // u is not mostly along X
                    c1r = -ug; c1g = ur; c1b = 0.0; // Cross with Z-axis (0,0,1) like (u.y, -u.x, 0)
                }
                 double mag_c1_fallback = std::hypot(c1r, c1g, c1b);
                 if (mag_c1_fallback > default_Tolerance) { c1r /= mag_c1_fallback; c1g /= mag_c1_fallback; c1b /= mag_c1_fallback;}
                 else { c1r = 0; c1g = (std::abs(ur) < 0.9 ? 1.0 : 0.0); c1b = (std::abs(ur) < 0.9 ? 0.0 : 1.0);} // Default if still zero
            }

            // v_new_rgb = cross_product(c1_rgb, u_rgb)
            double vr_new = c1g * ub - c1b * ug;
            double vg_new = c1b * ur - c1r * ub;
            double vb_new = c1r * ug - c1g * ur;

            // Normalize v_new_rgb (should be auto-normalized if c1 and u were orthonormal, but for safety)
            double mag_v_new = std::hypot(vr_new, vg_new, vb_new);
            if (mag_v_new > default_Tolerance) {
                vr_new /= mag_v_new; vg_new /= mag_v_new; vb_new /= mag_v_new;
            } else { // Should not happen if c1 and u are ortho and non-zero.
                 // This indicates c1 and u were parallel, which means original v was parallel to u,
                 // and the fallback for c1 also resulted in parallel to u. This is an edge case.
                 // Default v_new_rgb to be orthogonal to u_rgb.
                 // If ur,ug,ub is (1,0,0), vr_new,vg_new,vb_new could be (0,1,0).
                if (std::abs(ur) > 0.9) { vr_new=0; vg_new=1; vb_new=0; }
                else if (std::abs(ug) > 0.9) { vr_new=1; vg_new=0; vb_new=0; }
                else { vr_new=0; vg_new=0; vb_new=1; } // Fallback if not axis aligned
                // A more robust way to find a perpendicular if all else fails:
                if (std::abs(ur) > default_Tolerance || std::abs(ug) > default_Tolerance) { // If not (0,0,Z)
                    vr_new = -ug; vg_new = ur; vb_new = 0;
                } else { // Is (0,0,Z) or (0,0,0)
                    vr_new = 1; vg_new = 0; vb_new = 0;
                }
                double mag_v_fallback = std::hypot(vr_new,vg_new,vb_new);
                if(mag_v_fallback > default_Tolerance) {vr_new/=mag_v_fallback; vg_new/=mag_v_fallback; vb_new/=mag_v_fallback;}

            }

            u.r = ur; u.g = ug; u.b = ub; u.a = u_alpha;
            v.r = vr_new; v.g = vg_new; v.b = vb_new; v.a = v_alpha;
        }


        // --- Operators (Apply to all components R,G,B,A) ---
        [[nodiscard]] vecRGBA operator+(const vecRGBA& other) const { return {r+other.r, g+other.g, b+other.b, a+other.a}; }
        [[nodiscard]] vecRGBA operator-(const vecRGBA& other) const { return {r-other.r, g-other.g, b-other.b, a-other.a}; }
        // Component-wise multiplication (Modulate for RGBA)
        [[nodiscard]] vecRGBA operator*(const vecRGBA& other) const { return {r*other.r, g*other.g, b*other.b, a*other.a}; }
        // Component-wise division (for RGBA)
        [[nodiscard]] vecRGBA operator/(const vecRGBA& other) const {
            return {safe_divide(r, other.r), safe_divide(g, other.g), safe_divide(b, other.b), safe_divide(a, other.a)};
        }

        [[nodiscard]] vecRGBA operator-() const { return {-r, -g, -b, -a}; } // Negation of all components
        [[nodiscard]] vecRGBA operator+(const double& scalar) const { return {r+scalar, g+scalar, b+scalar, a+scalar}; }
        [[nodiscard]] vecRGBA operator-(const double& scalar) const { return {r-scalar, g-scalar, b-scalar, a-scalar}; }
        [[nodiscard]] vecRGBA operator*(const double& scalar) const { return {r*scalar, g*scalar, b*scalar, a*scalar}; } // Scales all components
        [[nodiscard]] vecRGBA operator/(const double& scalar) const {
            return {safe_divide(r, scalar), safe_divide(g, scalar), safe_divide(b, scalar), safe_divide(a, scalar)};
        }

        vecRGBA& operator+=(const vecRGBA& other) { r+=other.r; g+=other.g; b+=other.b; a+=other.a; return *this; }
        vecRGBA& operator-=(const vecRGBA& other) { r-=other.r; g-=other.g; b-=other.b; a-=other.a; return *this; }
        vecRGBA& operator*=(const vecRGBA& other) { r*=other.r; g*=other.g; b*=other.b; a*=other.a; return *this; }
        vecRGBA& operator/=(const vecRGBA& other) {
            r=safe_divide(r,other.r); g=safe_divide(g,other.g); b=safe_divide(b,other.b); a=safe_divide(a,other.a); return *this;
        }

        vecRGBA& operator+=(const double& scalar) { r+=scalar; g+=scalar; b+=scalar; a+=scalar; return *this; }
        vecRGBA& operator-=(const double& scalar) { r-=scalar; g-=scalar; b-=scalar; a-=scalar; return *this; }
        vecRGBA& operator*=(const double& scalar) { r*=scalar; g*=scalar; b*=scalar; a*=scalar; return *this; }
        vecRGBA& operator/=(const double& scalar) {
            r=safe_divide(r,scalar); g=safe_divide(g,scalar); b=safe_divide(b,scalar); a=safe_divide(a,scalar); return *this;
        }

        // Equality: Compares R,G,B,A using exact comparison.
        bool operator==(const vecRGBA& other) const {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }
        bool operator!=(const vecRGBA& other) const { return !(*this == other); }

        // Comparison operators (based on RGB magnitude, geometric interpretation). Alpha ignored for ordering.
        bool operator<(const vecRGBA& other) const { return magnitudeSquaredRGB() < other.magnitudeSquaredRGB(); }
        bool operator<=(const vecRGBA& other) const { return magnitudeSquaredRGB() <= other.magnitudeSquaredRGB(); }
        bool operator>(const vecRGBA& other) const { return magnitudeSquaredRGB() > other.magnitudeSquaredRGB(); }
        bool operator>=(const vecRGBA& other) const { return magnitudeSquaredRGB() >= other.magnitudeSquaredRGB(); }

        // --- String Conversion ---
        std::string toStr() const { return std::format("vecRGBA[r:{:.3f},g:{:.3f},b:{:.3f},a:{:.3f}]", r, g, b, a); }
    };

    // --- Non-member operators for symmetry (scalar on left, applies to R,G,B,A) ---
    [[nodiscard]] inline vecRGBA operator+(const double& scalar, const vecRGBA& v) { return v + scalar; }
    [[nodiscard]] inline vecRGBA operator-(const double& scalar, const vecRGBA& v) { return {scalar-v.r, scalar-v.g, scalar-v.b, scalar-v.a}; }
    [[nodiscard]] inline vecRGBA operator*(const double& scalar, const vecRGBA& v) { return v * scalar; }
    [[nodiscard]] inline vecRGBA operator/(const double& scalar, const vecRGBA& v) {
        return {safe_divide(scalar, v.r), safe_divide(scalar, v.g), safe_divide(scalar, v.b), safe_divide(scalar, v.a)};
    }

    inline std::ostream& operator<<(std::ostream& os, const vecRGBA& v) {
        os << v.toStr();
        return os;
    }

} // End of namespace vecSys