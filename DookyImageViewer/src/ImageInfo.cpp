#include "ImageInfo.h"

#include <time.h>
#include <fstream>

namespace Dooky {
    size_t GetFileLastModifiedTime(const std::filesystem::path& path) {
        auto lastModifiedTime = std::filesystem::last_write_time(path);

        auto tt = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastModifiedTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        auto tt2 = std::chrono::system_clock::to_time_t(tt);

        return tt2;
    }

    std::string GetFileLastModifiedTimestampString(const std::filesystem::path& path) {
        std::string timestamp;

        auto lastModifiedTime = std::filesystem::last_write_time(path);

        auto tt = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastModifiedTime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        auto tt2 = std::chrono::system_clock::to_time_t(tt);
        
        std::tm* gmt;
        gmt = localtime(&tt2);

        std::stringstream buffer;
        std::stringstream pmBuffer;
        pmBuffer << std::put_time(gmt, "%H");

        bool isPM = std::stoi(pmBuffer.str()) > 12;

        buffer << std::put_time(gmt, "%a, %d %b %Y %I:%M:%S ");
        buffer << (isPM ? "PM" : "AM");

        return buffer.str();
    }

    TinyEXIF::EXIFInfo GetImageExifData(const std::filesystem::path& path) {
        std::ifstream input(path, std::ifstream::binary);
        TinyEXIF::EXIFInfo imageEXIF(input);

        return imageEXIF;
    }

    std::string GetImageExifInformationString(const TinyEXIF::EXIFInfo& imageEXIF) {
        std::stringstream exifText;

        if (imageEXIF.Fields) {
            if (imageEXIF.ImageWidth || imageEXIF.ImageHeight)
                exifText << "Image Resolution: " << imageEXIF.ImageWidth << "x" << imageEXIF.ImageHeight << "\n";

            if (imageEXIF.RelatedImageWidth || imageEXIF.RelatedImageHeight)
                exifText << "Related Image Resolution: " << imageEXIF.RelatedImageWidth << "x" << imageEXIF.RelatedImageHeight << "\n";

            if (!imageEXIF.Make.empty() || !imageEXIF.Model.empty())
                exifText << "Camera Model: " << imageEXIF.Make << " - " << imageEXIF.Model << "\n";

            if (!imageEXIF.SerialNumber.empty())
                exifText << "Serial Number: " << imageEXIF.SerialNumber << "\n";

            if (imageEXIF.Orientation)
                exifText << "Orientation: " << imageEXIF.Orientation << "\n";

            if (imageEXIF.XResolution || imageEXIF.YResolution || imageEXIF.ResolutionUnit) {
                std::string unit = "Other";

                switch (imageEXIF.ResolutionUnit) {
                case 1: unit = "No unit"; break;
                case 2: unit = "Dots Per Inch"; break;
                case 3: unit = "Dots Per CM"; break;
                }

                exifText << "Resolution: " << imageEXIF.XResolution << "x" << imageEXIF.YResolution << " (" << unit << ")\n";
            }

            if (imageEXIF.BitsPerSample)
                exifText << "Bits Per Sample: " << imageEXIF.BitsPerSample << "\n";

            if (!imageEXIF.Software.empty())
                exifText << "Software: " << imageEXIF.Software << "\n";

            if (!imageEXIF.DateTime.empty())
                exifText << "DateTime: " << imageEXIF.DateTime << "\n";

            if (!imageEXIF.DateTimeOriginal.empty())
                exifText << "DateTimeOriginal: " << imageEXIF.DateTimeOriginal << "\n";

            if (!imageEXIF.DateTimeDigitized.empty())
                exifText << "DateTimeDigitized: " << imageEXIF.DateTimeDigitized << "\n";

            if (!imageEXIF.SubSecTimeOriginal.empty())
                exifText << "SubSecTimeOriginal: " << imageEXIF.SubSecTimeOriginal << "\n";

            if (!imageEXIF.Copyright.empty())
                exifText << "Copyright: " << imageEXIF.Copyright << "\n";

            exifText << "\n";

            { // Exposure Time
                if (imageEXIF.ExposureTime < 1) {
                    exifText << "Exposure Time: " << std::setprecision(10) << imageEXIF.ExposureTime << "s or " << "(1/" << (1 / imageEXIF.ExposureTime) << "s)\n";
                } else {
                    exifText << "Exposure Time: " << std::setprecision(10) << imageEXIF.ExposureTime << "s\n";
                }
            }
        
            exifText << "ISO Speed: " << imageEXIF.ISOSpeedRatings << "\n";
            exifText << "FNumber: f" << imageEXIF.FNumber << "\n";
        
            exifText << "\n";
         
            { // Exposure Program
                uint16_t code = imageEXIF.ExposureProgram;
                std::string exposureStr = "Not defined";

                switch (code) {
                case 1: exposureStr = "Manual"; break;
                case 2: exposureStr = "Normal program"; break;
                case 3: exposureStr = "Aperture priority"; break;
                case 4: exposureStr = "Shutter priority"; break;
                case 5: exposureStr = "Creative program (biased toward depth of field)"; break;
                case 6: exposureStr = "Action program (biased toward fast shutter speed)"; break;
                case 7: exposureStr = "Portrait mode (for closeup photos with the background out of focus)"; break;
                case 8: exposureStr = "Landscape mode (for landscape photos with the background in focus)"; break;
                }

                exifText << "Exposure Program: " << code << " (" << exposureStr << ")\n";
            }
        
            exifText << "Shutter Speed Value: " << std::setprecision(10) << imageEXIF.ShutterSpeedValue << "\n";
            exifText << "Aperture Value: " << std::setprecision(10) << imageEXIF.ApertureValue << "\n";
            exifText << "Brightness Value: " << std::setprecision(10) << imageEXIF.BrightnessValue << "\n";
            exifText << "ExposureBias Value: " << imageEXIF.ExposureBiasValue << "\n";
            exifText << "Focal Length: " << imageEXIF.FocalLength << "mm\n";

            { // Flash
                uint16_t code = imageEXIF.Flash;
                std::string flashStr = "Unknown";

                switch (code) {
                case 0x00: flashStr = "Flash did not fire"; break;
                case 0x01: flashStr = "Flash fired"; break;
                case 0x05: flashStr = "Strobe return light not detected"; break;
                case 0x07: flashStr = "Strobe return light detected"; break;
                case 0x09: flashStr = "Flash fired, compulsory flash mode"; break;
                case 0x0D: flashStr = "Flash fired, compulsory flash mode, return light not detected"; break;
                case 0x0F: flashStr = "Flash fired, compulsory flash mode, return light detected"; break;
                case 0x10: flashStr = "Flash did not fire, compulsory flash mode"; break;
                case 0x18: flashStr = "Flash did not fire, auto mode"; break;
                case 0x19: flashStr = "Flash fired, auto mode"; break;
                case 0x1D: flashStr = "Flash fired, auto mode, return light not detected"; break;
                case 0x1F: flashStr = "Flash fired, auto mode, return light detected"; break;
                case 0x20: flashStr = "No flash function"; break;
                case 0x41: flashStr = "Flash fired, red-eye reduction mode"; break;
                case 0x45: flashStr = "Flash fired, red-eye reduction mode, return light not detected"; break;
                case 0x47: flashStr = "Flash fired, red-eye reduction mode, return light detected"; break;
                case 0x49: flashStr = "Flash fired, compulsory flash mode, red-eye reduction mode"; break;
                case 0x4D: flashStr = "Flash fired, compulsory flash mode, red-eye reduction mode, return light not detected"; break;
                case 0x4F: flashStr = "Flash fired, compulsory flash mode, red-eye reduction mode, return light detected"; break;
                case 0x59: flashStr = "Flash fired, auto mode, red-eye reduction mode"; break;
                case 0x5D: flashStr = "Flash fired, auto mode, return light not detected, red-eye reduction mode"; break;
                case 0x5F: flashStr = "Flash fired, auto mode, return light detected, red-eye reduction mode"; break;
                }

                exifText << "Flash: " << code << " (" << flashStr << ")\n";
            }

            exifText << "Subject Distance: " << imageEXIF.SubjectDistance << "\n";

            if (!imageEXIF.SubjectArea.empty()) {
                exifText << "SubjectArea: ";

                for (auto val : imageEXIF.SubjectArea) {
                    exifText << " " << val;
                }

                exifText << "\n";
            }

            exifText << "\n";

            { // MeteringMode
                uint16_t code = imageEXIF.MeteringMode;
                std::string meteringModeStr = "Unknown";

                switch (code) {
                case 1: meteringModeStr = "Average"; break;
                case 2: meteringModeStr = "CenterWeightedAverage"; break;
                case 3: meteringModeStr = "Spot"; break;
                case 4: meteringModeStr = "MultiSpot"; break;
                case 5: meteringModeStr = "Pattern"; break;
                case 6: meteringModeStr = "Partial"; break;
                case 255: meteringModeStr = "Other"; break;
                }

                exifText << "MeteringMode: " << code << " (" << meteringModeStr << ")\n";
            }

            { // LightSource
                uint16_t code = imageEXIF.LightSource;
                std::string lightSourceStr = "Unknown";

                switch (code) {
                case 1: lightSourceStr = "Daylight"; break;
                case 2: lightSourceStr = "Fluorescent"; break;
                case 3: lightSourceStr = "Tungsten (incandescent light)"; break;
                case 4: lightSourceStr = "Flash"; break;
                case 9: lightSourceStr = "Fine weather"; break;
                case 10: lightSourceStr = "Cloudy weather"; break;
                case 11: lightSourceStr = "Shade"; break;
                case 12: lightSourceStr = "Daylight fluorescent (D 5700 - 7100K)"; break;
                case 13: lightSourceStr = "Day white fluorescent (N 4600 - 5400K)"; break;
                case 14: lightSourceStr = "Cool white fluorescent (W 3900 - 4500K)"; break;
                case 15: lightSourceStr = "White fluorescent (WW 3200 - 3700K)"; break;
                case 17: lightSourceStr = "Standard light A"; break;
                case 18: lightSourceStr = "Standard light B"; break;
                case 19: lightSourceStr = "Standard light C"; break;
                case 20: lightSourceStr = "D55"; break;
                case 21: lightSourceStr = "D65"; break;
                case 22: lightSourceStr = "D75"; break;
                case 23: lightSourceStr = "D50"; break;
                case 24: lightSourceStr = "ISO studio tungsten"; break;
                case 255: lightSourceStr = "Other light source"; break;
                }

                exifText << "LightSource: " << code << " (" << lightSourceStr << ")\n";
            }

            exifText << "ProjectionType: " << imageEXIF.ProjectionType << "\n";

            exifText << "\n";
        
            if (imageEXIF.Calibration.FocalLength != 0)
                exifText << "Calibration FocalLength: " << imageEXIF.Calibration.FocalLength << "pixels\n";

            if (imageEXIF.Calibration.OpticalCenterX != 0)
                exifText << "Calibration OpticalCenterX: " << imageEXIF.Calibration.OpticalCenterX << "pixels\n";

            if (imageEXIF.Calibration.OpticalCenterY != 0)
                exifText << "Calibration OpticalCenterY: " << imageEXIF.Calibration.OpticalCenterY << "pixels\n";

            // Lens information

            exifText << "\n";

            exifText << "Lens FStopMin: " << imageEXIF.LensInfo.FStopMin << "\n";
            exifText << "Lens FStopMax: " << imageEXIF.LensInfo.FStopMax << "\n";
            exifText << "Lens FocalLengthMin: " << imageEXIF.LensInfo.FocalLengthMin << "mm\n";
            exifText << "Lens FocalLengthMax: " << imageEXIF.LensInfo.FocalLengthMax << "mm\n";
            exifText << "Lens DigitalZoomRatio: " << imageEXIF.LensInfo.DigitalZoomRatio << "\n";
            exifText << "Lens FocalLengthIn35mm: " << imageEXIF.LensInfo.FocalLengthIn35mm << "\n";
            exifText << "Lens FocalPlaneXResolution: " << imageEXIF.LensInfo.FocalPlaneXResolution << "\n";
            exifText << "Lens FocalPlaneYResolution: " << imageEXIF.LensInfo.FocalPlaneYResolution << "\n";

            {
                std::string unit = "Other";

                switch (imageEXIF.ResolutionUnit) {
                case 1: unit = "No unit"; break;
                case 2: unit = "Inch"; break;
                case 3: unit = "CM"; break;
                }

                exifText << "Lens FocalPlaneResolutionUnit: " << imageEXIF.LensInfo.FocalPlaneResolutionUnit << " (" << unit << ")\n";
            }

            if (!imageEXIF.LensInfo.Make.empty() || !imageEXIF.LensInfo.Model.empty())
                exifText << "Lens Model: " << imageEXIF.LensInfo.Make << " - " << imageEXIF.LensInfo.Model << "\n";

            exifText << "\n";

            // Geolocation
        
            if (imageEXIF.GeoLocation.hasLatLon()) {
                exifText << "GeoLocation Latitude: " << std::setprecision(10) << imageEXIF.GeoLocation.Latitude << "\n";
                exifText << "GeoLocation Longitude: " << std::setprecision(10) << imageEXIF.GeoLocation.Longitude << "\n";
            }
        
            if (imageEXIF.GeoLocation.hasAltitude()) {
                exifText << "GeoLocation Altitude: " << imageEXIF.GeoLocation.Altitude << "m \n";
                exifText << "GeoLocation AltitudeRef: " << (int)imageEXIF.GeoLocation.AltitudeRef << "\n";
            }

            if (imageEXIF.GeoLocation.hasRelativeAltitude()) {
                exifText << "GeoLocation RelativeAltitude: " << imageEXIF.GeoLocation.RelativeAltitude << "m\n";
            }

            if (imageEXIF.GeoLocation.hasOrientation()) {
                exifText << "GeoLocation RollDegree: " << imageEXIF.GeoLocation.RollDegree << "\n";
                exifText << "GeoLocation PitchDegree: " << imageEXIF.GeoLocation.PitchDegree << "\n";
                exifText << "GeoLocation YawDegree: " << imageEXIF.GeoLocation.YawDegree << "\n";
            }

            if (imageEXIF.GeoLocation.hasSpeed()) {
                exifText << "GeoLocation SpeedX: " << imageEXIF.GeoLocation.SpeedX << "m/s\n";
                exifText << "GeoLocation SpeedY: " << imageEXIF.GeoLocation.SpeedY << "m/s\n";
                exifText << "GeoLocation SpeedZ: " << imageEXIF.GeoLocation.SpeedZ << "m/s\n";
            }

            if (imageEXIF.GeoLocation.AccuracyXY > 0 || imageEXIF.GeoLocation.AccuracyZ > 0) {
                exifText << "GeoLocation GPSAccuracy XY: " << imageEXIF.GeoLocation.AccuracyXY << "m Z: " << imageEXIF.GeoLocation.AccuracyZ << "m\n";
            }

            exifText << "GeoLocation GPSDOP: " << imageEXIF.GeoLocation.GPSDOP << "\n";
            exifText << "GeoLocation GPS Differential: " << imageEXIF.GeoLocation.GPSDifferential << "\n";

            if (!imageEXIF.GeoLocation.GPSMapDatum.empty())
                exifText << "GeoLocation GPS Map Datum: " << imageEXIF.GeoLocation.GPSMapDatum << "\n";

            if (!imageEXIF.GeoLocation.GPSTimeStamp.empty())
                exifText << "GeoLocation GPS Time Stamp: " << imageEXIF.GeoLocation.GPSTimeStamp << "\n";

            if (!imageEXIF.GeoLocation.GPSDateStamp.empty())
                exifText << "GeoLocation GPS Date Stamp: " << imageEXIF.GeoLocation.GPSDateStamp << "\n";

            exifText << "\n";

            if (imageEXIF.GPano.hasPosePitchDegrees())
                exifText << "GPano PosePitchDegrees: " << imageEXIF.GPano.PosePitchDegrees << "\n";

            if (imageEXIF.GPano.hasPoseRollDegrees())
                exifText << "GPano PoseRollDegrees: " << imageEXIF.GPano.PoseRollDegrees << "\n";

            exifText << "\n";

            if (!imageEXIF.ImageDescription.empty())
                exifText << "Description: " << imageEXIF.ImageWidth << "x" << imageEXIF.ImageHeight << "\n";
        }

        return exifText.str();
    }
}