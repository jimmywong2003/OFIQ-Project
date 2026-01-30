/**
 * @file icao_compliance_checker.cpp
 *
 * @brief Console application for checking ICAO 9303 compliance using OFIQ library
 *
 * @copyright Copyright (c) 2024  Federal Office for Information Security, Germany
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @author OFIQ development team
 */

#include "image_io.h"
#include "ofiq_lib.h"
#include "utils.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <magic_enum.hpp>
#include <tao/json.hpp>
#include <string>
#include <vector>

using json = tao::json::value;
namespace fs = std::filesystem;

using namespace std;
using namespace OFIQ;
using namespace OFIQ_LIB;

// Constants
constexpr int SUCCESS = 0;
constexpr int FAILURE = 1;

// Structure to hold ICAO compliance thresholds
struct ICAOThreshold
{
    double min = 0.0;
    double max = 100.0;
    string description;
};

// Function declarations
int processImage(const shared_ptr<Interface> &implPtr, const string &imagePath,
                 const map<string, ICAOThreshold> &thresholds, json &output);
map<string, ICAOThreshold> loadThresholds(const string &configPath);
void printUsage(const string &executable);
string measureEnumToString(QualityMeasure measure);
json extractFacialLandmarks(const OFIQ::FaceLandmarks &landmarks);

int main(int argc, char *argv[])
{
    // Default paths
    fs::path imagePath;
    fs::path configDir = ".";
    fs::path thresholdsPath = "icao_compliance_config.json";
    fs::path outputPath;

    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--image") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "ERROR: Image path missing after -i/--image" << endl;
                printUsage(argv[0]);
                return FAILURE;
            }
            imagePath = fs::path(argv[++i]);
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config-dir") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "ERROR: Config directory missing after -c/--config-dir" << endl;
                printUsage(argv[0]);
                return FAILURE;
            }
            configDir = fs::path(argv[++i]);
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--thresholds") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "ERROR: Thresholds file missing after -t/--thresholds" << endl;
                printUsage(argv[0]);
                return FAILURE;
            }
            thresholdsPath = fs::path(argv[++i]);
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "ERROR: Output file missing after -o/--output" << endl;
                printUsage(argv[0]);
                return FAILURE;
            }
            outputPath = fs::path(argv[++i]);
        }
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printUsage(argv[0]);
            return SUCCESS;
        }
        else
        {
            cerr << "ERROR: Unrecognized option: " << argv[i] << endl;
            printUsage(argv[0]);
            return FAILURE;
        }
    }

    // Validate required arguments
    if (imagePath.empty())
    {
        cerr << "ERROR: Image path is required" << endl;
        printUsage(argv[0]);
        return FAILURE;
    }

    if (!fs::exists(imagePath))
    {
        cerr << "ERROR: Image file does not exist: " << imagePath << endl;
        return FAILURE;
    }

    // Load ICAO compliance thresholds
    cout << "Loading ICAO compliance thresholds from: " << thresholdsPath << endl;
    map<string, ICAOThreshold> thresholds;
    try
    {
        thresholds = loadThresholds(thresholdsPath.string());
        cout << "Loaded thresholds for " << thresholds.size() << " measures" << endl;
    }
    catch (const exception &e)
    {
        cerr << "ERROR: Failed to load thresholds: " << e.what() << endl;
        cerr << "Using default thresholds (min=50, max=100)" << endl;
        // Continue with empty thresholds - will use defaults
    }

    // Get OFIQ implementation
    auto implPtr = Interface::getImplementation();
    if (!implPtr)
    {
        cerr << "ERROR: Failed to get OFIQ implementation" << endl;
        return FAILURE;
    }

    // Initialize OFIQ library
    fs::path ofiqConfigFile = "ofiq_config.jaxn";
    if (!fs::exists(configDir / ofiqConfigFile))
    {
        // Try data directory as fallback
        if (fs::exists("data/ofiq_config.jaxn"))
        {
            configDir = "data";
        }
        else
        {
            cerr << "ERROR: OFIQ configuration file not found in " << configDir
                 << " or data directory" << endl;
            return FAILURE;
        }
    }

    cout << "Initializing OFIQ library with config: " << (configDir / ofiqConfigFile) << endl;
    auto startInit = chrono::high_resolution_clock::now();
    auto ret = implPtr->initialize(configDir.string(), ofiqConfigFile.string());
    auto endInit = chrono::high_resolution_clock::now();
    auto initTime = chrono::duration_cast<chrono::milliseconds>(endInit - startInit);

    if (ret.code != ReturnCode::Success)
    {
        cerr << "ERROR: OFIQ initialization failed: " << ret.code << endl;
        if (!ret.info.empty())
        {
            cerr << "Additional info: " << ret.info << endl;
        }
        return FAILURE;
    }

    cout << "OFIQ initialization completed in " << initTime.count() << " ms" << endl;

    // Get version info
    int major, minor, patch;
    implPtr->getVersion(major, minor, patch);
    cout << "OFIQ library version: " << major << "." << minor << "." << patch << endl;

    // Process the image
    json output;
    int result = processImage(implPtr, imagePath.string(), thresholds, output);

    // Add metadata
    output["image"] = imagePath.string();
    output["ofiq_version"] = to_string(major) + "." + to_string(minor) + "." + to_string(patch);
    output["init_time_ms"] = initTime.count();
    output["thresholds_file"] = thresholdsPath.string();

    // Output results
    if (outputPath.empty())
    {
        cout << tao::json::to_string(output, 2) << endl;
    }
    else
    {
        ofstream outFile(outputPath);
        if (outFile.good())
        {
            outFile << tao::json::to_string(output, 2) << endl;
            cout << "Results written to: " << outputPath << endl;
        }
        else
        {
            cerr << "ERROR: Failed to write output to: " << outputPath << endl;
            cout << tao::json::to_string(output, 2) << endl;
        }
    }

    return result;
}

int processImage(const shared_ptr<Interface> &implPtr, const string &imagePath,
                 const map<string, ICAOThreshold> &thresholds, json &output)
{
    // Read image
    Image image;
    auto startRead = chrono::high_resolution_clock::now();
    ReturnStatus retStatus = readImage(imagePath, image);
    auto endRead = chrono::high_resolution_clock::now();
    auto readTime = chrono::duration_cast<chrono::milliseconds>(endRead - startRead);

    if (retStatus.code != ReturnCode::Success)
    {
        output["error"] = "Failed to read image: " + retStatus.info;
        cerr << "ERROR: " << retStatus.info << endl;
        return FAILURE;
    }

    cout << "Image loaded: " << image.width << "x" << image.height
         << " (depth: " << (int)image.depth << ") in " << readTime.count() << " ms" << endl;

    // Process image with OFIQ - request landmarks as well
    FaceImageQualityAssessment assessments;
    FaceImageQualityPreprocessingResult preprocessingResult;
    auto startProcess = chrono::high_resolution_clock::now();

    // Request landmarks using the preprocessing results API
    uint32_t resultRequestsMask = static_cast<uint32_t>(PreprocessingResultType::Landmarks);
    retStatus = implPtr->vectorQualityWithPreprocessingResults(
        image, assessments, preprocessingResult, resultRequestsMask);

    auto endProcess = chrono::high_resolution_clock::now();
    auto processTime = chrono::duration_cast<chrono::milliseconds>(endProcess - startProcess);

    if (retStatus.code != ReturnCode::Success)
    {
        output["error"] = "Failed to assess image quality: " + retStatus.info;
        cerr << "ERROR: Quality assessment failed: " << retStatus.code << endl;
        if (!retStatus.info.empty())
        {
            cerr << "Additional info: " << retStatus.info << endl;
        }
        return FAILURE;
    }

    cout << "Quality assessment completed in " << processTime.count() << " ms" << endl;
    cout << "Number of measures computed: " << assessments.qAssessments.size() << endl;

    // Extract face detection and pose information
    json faceInfoJson = json({});

    // Extract and display facial landmarks if available
    if (preprocessingResult.m_landmarks.type == LandmarkType::LM_98 &&
        !preprocessingResult.m_landmarks.landmarks.empty())
    {
        cout << "Facial landmarks detected: " << preprocessingResult.m_landmarks.landmarks.size() << " points" << endl;

        // Extract key facial features
        json landmarksJson = extractFacialLandmarks(preprocessingResult.m_landmarks);
        faceInfoJson["landmarks"] = landmarksJson;

        // Display key landmark positions
        if (landmarksJson.is_object())
        {
            const auto &landmarksObj = landmarksJson.get_object();

            auto leftEyeIt = landmarksObj.find("left_eye");
            if (leftEyeIt != landmarksObj.end() && leftEyeIt->second.is_object())
            {
                const auto &leftEyeObj = leftEyeIt->second.get_object();
                auto centerIt = leftEyeObj.find("center");
                if (centerIt != leftEyeObj.end() && centerIt->second.is_object())
                {
                    const auto &centerObj = centerIt->second.get_object();
                    auto xIt = centerObj.find("x");
                    auto yIt = centerObj.find("y");
                    if (xIt != centerObj.end() && yIt != centerObj.end() &&
                        xIt->second.is_number() && yIt->second.is_number())
                    {
                        cout << "Left eye center: (" << xIt->second.get_double()
                             << ", " << yIt->second.get_double() << ")" << endl;
                    }
                }
            }

            auto rightEyeIt = landmarksObj.find("right_eye");
            if (rightEyeIt != landmarksObj.end() && rightEyeIt->second.is_object())
            {
                const auto &rightEyeObj = rightEyeIt->second.get_object();
                auto centerIt = rightEyeObj.find("center");
                if (centerIt != rightEyeObj.end() && centerIt->second.is_object())
                {
                    const auto &centerObj = centerIt->second.get_object();
                    auto xIt = centerObj.find("x");
                    auto yIt = centerObj.find("y");
                    if (xIt != centerObj.end() && yIt != centerObj.end() &&
                        xIt->second.is_number() && yIt->second.is_number())
                    {
                        cout << "Right eye center: (" << xIt->second.get_double()
                             << ", " << yIt->second.get_double() << ")" << endl;
                    }
                }
            }

            auto noseIt = landmarksObj.find("nose_tip");
            if (noseIt != landmarksObj.end() && noseIt->second.is_object())
            {
                const auto &noseObj = noseIt->second.get_object();
                auto xIt = noseObj.find("x");
                auto yIt = noseObj.find("y");
                if (xIt != noseObj.end() && yIt != noseObj.end())
                {
                    try
                    {
                        // Check if they're numbers before accessing
                        if (xIt->second.is_number() && yIt->second.is_number())
                        {
                            double x = xIt->second.get_double();
                            double y = yIt->second.get_double();
                            cout << "Nose tip: (" << x << ", " << y << ")" << endl;
                        }
                        else
                        {
                            cout << "Nose tip: (not a valid number)" << endl;
                        }
                    }
                    catch (const exception &e)
                    {
                        cout << "Nose tip: (error accessing coordinates)" << endl;
                    }
                }
                else
                {
                    cout << "Nose tip: (coordinates not found)" << endl;
                }
            }
            else
            {
                cout << "Nose tip: (not detected)" << endl;
            }

            auto mouthIt = landmarksObj.find("mouth_center");
            if (mouthIt != landmarksObj.end() && mouthIt->second.is_object())
            {
                const auto &mouthObj = mouthIt->second.get_object();
                auto xIt = mouthObj.find("x");
                auto yIt = mouthObj.find("y");
                if (xIt != mouthObj.end() && yIt != mouthObj.end() &&
                    xIt->second.is_number() && yIt->second.is_number())
                {
                    cout << "Mouth center: (" << xIt->second.get_double()
                         << ", " << yIt->second.get_double() << ")" << endl;
                }
            }
        }
    }
    else
    {
        faceInfoJson["landmarks"] = tao::json::null;
        cout << "No facial landmarks detected" << endl;
    }

    // Face bounding box
    if (assessments.boundingBox.xleft >= 0 && assessments.boundingBox.ytop >= 0 &&
        assessments.boundingBox.width > 0 && assessments.boundingBox.height > 0)
    {
        json bboxJson = json({});
        bboxJson["x"] = assessments.boundingBox.xleft;
        bboxJson["y"] = assessments.boundingBox.ytop;
        bboxJson["width"] = assessments.boundingBox.width;
        bboxJson["height"] = assessments.boundingBox.height;
        bboxJson["detector_type"] = static_cast<int>(assessments.boundingBox.faceDetector);
        faceInfoJson["bounding_box"] = bboxJson;

        cout << "Face detected at: x=" << assessments.boundingBox.xleft
             << ", y=" << assessments.boundingBox.ytop
             << ", width=" << assessments.boundingBox.width
             << ", height=" << assessments.boundingBox.height << endl;
    }
    else
    {
        faceInfoJson["bounding_box"] = tao::json::null;
        cout << "No valid face bounding box detected" << endl;
    }

    // Extract pose information from quality measures
    json poseJson = json({});
    bool hasPoseInfo = false;

    for (const auto &[measure, result] : assessments.qAssessments)
    {
        if (measure == QualityMeasure::HeadPoseYaw && result.code == QualityMeasureReturnCode::Success)
        {
            poseJson["yaw"] = result.rawScore;
            hasPoseInfo = true;
        }
        else if (measure == QualityMeasure::HeadPosePitch && result.code == QualityMeasureReturnCode::Success)
        {
            poseJson["pitch"] = result.rawScore;
            hasPoseInfo = true;
        }
        else if (measure == QualityMeasure::HeadPoseRoll && result.code == QualityMeasureReturnCode::Success)
        {
            poseJson["roll"] = result.rawScore;
            hasPoseInfo = true;
        }
    }

    if (hasPoseInfo)
    {
        faceInfoJson["pose"] = poseJson;

        // Get pose values for display
        string yawStr = "N/A";
        string pitchStr = "N/A";
        string rollStr = "N/A";

        if (poseJson.is_object())
        {
            const auto &poseObj = poseJson.get_object();
            auto yawIt = poseObj.find("yaw");
            if (yawIt != poseObj.end() && yawIt->second.is_number())
            {
                yawStr = to_string(yawIt->second.get_double());
            }

            auto pitchIt = poseObj.find("pitch");
            if (pitchIt != poseObj.end() && pitchIt->second.is_number())
            {
                pitchStr = to_string(pitchIt->second.get_double());
            }

            auto rollIt = poseObj.find("roll");
            if (rollIt != poseObj.end() && rollIt->second.is_number())
            {
                rollStr = to_string(rollIt->second.get_double());
            }
        }

        cout << "Head pose: yaw=" << yawStr
             << ", pitch=" << pitchStr
             << ", roll=" << rollStr << endl;
    }
    else
    {
        faceInfoJson["pose"] = tao::json::null;
        cout << "No pose information available" << endl;
    }

    // Prepare output JSON
    output["read_time_ms"] = readTime.count();
    output["process_time_ms"] = processTime.count();
    output["total_time_ms"] = readTime.count() + processTime.count();
    output["face_info"] = faceInfoJson;

    json measuresJson = json({});
    vector<string> failedMeasures;
    int passedCount = 0;
    int totalCount = 0;

    // Evaluate each measure against thresholds
    for (const auto &[measure, result] : assessments.qAssessments)
    {
        string measureName = measureEnumToString(measure);
        if (measureName.empty())
        {
            measureName = "Unknown_" + to_string(static_cast<int>(measure));
        }

        json measureJson;
        measureJson["raw_score"] = result.rawScore;
        measureJson["scalar_score"] = result.scalar;
        measureJson["status_code"] = static_cast<int>(result.code);

        // Check compliance
        bool isCompliant = false;
        if (result.code == QualityMeasureReturnCode::Success && result.scalar >= 0)
        {
            auto it = thresholds.find(measureName);
            if (it != thresholds.end())
            {
                const ICAOThreshold &threshold = it->second;
                measureJson["threshold_min"] = threshold.min;
                measureJson["threshold_max"] = threshold.max;
                measureJson["threshold_description"] = threshold.description;
                isCompliant = (result.scalar >= threshold.min && result.scalar <= threshold.max);
            }
            else
            {
                // Use default thresholds
                measureJson["threshold_min"] = 50.0;
                measureJson["threshold_max"] = 100.0;
                measureJson["threshold_description"] = "Default threshold";
                isCompliant = (result.scalar >= 50.0);
            }
        }
        else
        {
            // Failed to compute measure
            measureJson["threshold_min"] = tao::json::null;
            measureJson["threshold_max"] = tao::json::null;
            measureJson["threshold_description"] = "Measure computation failed";
            isCompliant = false;
        }

        measureJson["compliant"] = isCompliant;
        measuresJson[measureName] = measureJson;

        totalCount++;
        if (isCompliant)
        {
            passedCount++;
        }
        else
        {
            failedMeasures.push_back(measureName);
        }
    }

    output["measures"] = measuresJson;
    output["overall_compliance"] = (passedCount == totalCount);

    json summaryJson = json({});
    summaryJson["total_measures"] = totalCount;
    summaryJson["passed_measures"] = passedCount;
    summaryJson["failed_measures"] = totalCount - passedCount;

    // Convert vector to JSON array
    json failedMeasuresArray = json::array({});
    for (const auto &measure : failedMeasures)
    {
        failedMeasuresArray.push_back(measure);
    }
    summaryJson["failed_measure_names"] = failedMeasuresArray;

    output["summary"] = summaryJson;

    // Print summary to console
    cout << "\n=== ICAO Compliance Summary ===" << endl;
    cout << "Total measures: " << totalCount << endl;
    cout << "Passed: " << passedCount << endl;
    cout << "Failed: " << (totalCount - passedCount) << endl;
    cout << "Overall compliance: " << (passedCount == totalCount ? "PASS" : "FAIL") << endl;

    if (!failedMeasures.empty())
    {
        cout << "Failed measures: ";
        for (size_t i = 0; i < failedMeasures.size(); i++)
        {
            cout << failedMeasures[i];
            if (i < failedMeasures.size() - 1)
                cout << ", ";
        }
        cout << endl;
    }

    return SUCCESS;
}

map<string, ICAOThreshold> loadThresholds(const string &configPath)
{
    map<string, ICAOThreshold> thresholds;

    if (!fs::exists(configPath))
    {
        cerr << "WARNING: Thresholds file not found: " << configPath << endl;
        return thresholds;
    }

    try
    {
        // Read file content first for debugging
        ifstream file(configPath);
        if (!file.is_open())
        {
            throw runtime_error("Cannot open file: " + configPath);
        }

        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        file.close();

        cout << "Read " << content.size() << " bytes from thresholds file" << endl;

        // Debug: print first 200 chars of content
        if (content.size() > 0)
        {
            size_t previewSize = min(content.size(), (size_t)200);
            cout << "First " << previewSize << " chars: " << content.substr(0, previewSize) << endl;
        }

        // Use tao::json::from_string to parse JSON format
        json configJson;
        try
        {
            configJson = tao::json::from_string(content);
            cout << "JSON parsing successful" << endl;
        }
        catch (const exception &e)
        {
            cerr << "ERROR in tao::json::from_string: " << e.what() << endl;
            throw runtime_error("Invalid JSON format: " + string(e.what()));
        }

        // Check if the required section exists
        if (!configJson.is_object())
        {
            cerr << "DEBUG: configJson type: " << (configJson.is_object() ? "object" : configJson.is_array() ? "array"
                                                                                   : configJson.is_string()  ? "string"
                                                                                   : configJson.is_number()  ? "number"
                                                                                   : configJson.is_boolean() ? "boolean"
                                                                                                             : "null")
                 << endl;
            throw runtime_error("Config file is not a JSON object");
        }

        const auto &configObj = configJson.get_object();
        cout << "DEBUG: configObj has " << configObj.size() << " keys" << endl;

        // Debug: print all keys
        for (const auto &[key, value] : configObj)
        {
            cout << "  Key: " << key << ", Type: " << (value.is_object() ? "object" : value.is_array() ? "array"
                                                                                  : value.is_string()  ? "string"
                                                                                  : value.is_number()  ? "number"
                                                                                  : value.is_boolean() ? "boolean"
                                                                                                       : "null")
                 << endl;
        }

        auto thresholdsIt = configObj.find("icao_compliance_thresholds");
        if (thresholdsIt == configObj.end())
        {
            throw runtime_error("Missing 'icao_compliance_thresholds' section");
        }

        const auto &thresholdsJson = thresholdsIt->second;
        if (!thresholdsJson.is_object())
        {
            cerr << "DEBUG: thresholdsJson type: " << (thresholdsJson.is_object() ? "object" : thresholdsJson.is_array() ? "array"
                                                                                           : thresholdsJson.is_string()  ? "string"
                                                                                           : thresholdsJson.is_number()  ? "number"
                                                                                           : thresholdsJson.is_boolean() ? "boolean"
                                                                                                                         : "null")
                 << endl;
            throw runtime_error("'icao_compliance_thresholds' is not a JSON object");
        }

        const auto &thresholdsObj = thresholdsJson.get_object();
        cout << "DEBUG: thresholdsObj has " << thresholdsObj.size() << " measures" << endl;

        for (const auto &[measureName, thresholdJson] : thresholdsObj)
        {
            cout << "  Processing measure: " << measureName << endl;
            ICAOThreshold threshold;

            try
            {
                // Get min value with default
                if (thresholdJson.is_object())
                {
                    const auto &thresholdObj = thresholdJson.get_object();

                    // Debug: print all keys in this threshold object
                    cout << "    Threshold object keys: ";
                    for (const auto &[key, value] : thresholdObj)
                    {
                        cout << key << " ";
                    }
                    cout << endl;

                    auto minIt = thresholdObj.find("min");
                    if (minIt != thresholdObj.end())
                    {
                        try
                        {
                            // Check type and extract value accordingly
                            auto valueType = minIt->second.type();
                            if (valueType == tao::json::type::DOUBLE)
                            {
                                threshold.min = minIt->second.get_double();
                                cout << "    min: " << threshold.min << " (as double)" << endl;
                            }
                            else if (valueType == tao::json::type::SIGNED)
                            {
                                threshold.min = static_cast<double>(minIt->second.get_signed());
                                cout << "    min: " << threshold.min << " (converted from signed integer)" << endl;
                            }
                            else if (valueType == tao::json::type::UNSIGNED)
                            {
                                threshold.min = static_cast<double>(minIt->second.get_unsigned());
                                cout << "    min: " << threshold.min << " (converted from unsigned integer)" << endl;
                            }
                            else
                            {
                                cerr << "    WARNING: min is not a number, type: " << static_cast<int>(valueType) << endl;
                                threshold.min = 50.0; // default
                            }
                        }
                        catch (const exception &e)
                        {
                            cerr << "    ERROR accessing min value: " << e.what() << endl;
                            threshold.min = 50.0; // default
                        }
                    }
                    else
                    {
                        threshold.min = 50.0; // default
                        cout << "    min: default " << threshold.min << endl;
                    }

                    // Get max value with default
                    auto maxIt = thresholdObj.find("max");
                    if (maxIt != thresholdObj.end())
                    {
                        try
                        {
                            // Check type and extract value accordingly
                            auto valueType = maxIt->second.type();
                            if (valueType == tao::json::type::DOUBLE)
                            {
                                threshold.max = maxIt->second.get_double();
                                cout << "    max: " << threshold.max << " (as double)" << endl;
                            }
                            else if (valueType == tao::json::type::SIGNED)
                            {
                                threshold.max = static_cast<double>(maxIt->second.get_signed());
                                cout << "    max: " << threshold.max << " (converted from signed integer)" << endl;
                            }
                            else if (valueType == tao::json::type::UNSIGNED)
                            {
                                threshold.max = static_cast<double>(maxIt->second.get_unsigned());
                                cout << "    max: " << threshold.max << " (converted from unsigned integer)" << endl;
                            }
                            else
                            {
                                cerr << "    WARNING: max is not a number, type: " << static_cast<int>(valueType) << endl;
                                threshold.max = 100.0; // default
                            }
                        }
                        catch (const exception &e)
                        {
                            cerr << "    ERROR accessing max value: " << e.what() << endl;
                            threshold.max = 100.0; // default
                        }
                    }
                    else
                    {
                        threshold.max = 100.0; // default
                        cout << "    max: default " << threshold.max << endl;
                    }

                    // Get description with default
                    auto descIt = thresholdObj.find("description");
                    if (descIt != thresholdObj.end())
                    {
                        try
                        {
                            if (descIt->second.is_string())
                            {
                                threshold.description = descIt->second.get_string();
                                cout << "    description: " << threshold.description << endl;
                            }
                            else
                            {
                                cerr << "    WARNING: description is not a string" << endl;
                            }
                        }
                        catch (const exception &e)
                        {
                            cerr << "    ERROR accessing description: " << e.what() << endl;
                        }
                    }
                    else
                    {
                        cout << "    description: not found" << endl;
                    }
                }
                else
                {
                    // If not an object, use defaults
                    cerr << "DEBUG: thresholdJson for " << measureName << " is not an object, type: " << (thresholdJson.is_object() ? "object" : thresholdJson.is_array() ? "array"
                                                                                                                                             : thresholdJson.is_string()  ? "string"
                                                                                                                                             : thresholdJson.is_number()  ? "number"
                                                                                                                                             : thresholdJson.is_boolean() ? "boolean"
                                                                                                                                                                          : "null")
                         << endl;
                    threshold.min = 50.0;
                    threshold.max = 100.0;
                    threshold.description = "Default threshold";
                }
            }
            catch (const exception &e)
            {
                cerr << "    ERROR processing measure " << measureName << ": " << e.what() << endl;
                // Use defaults
                threshold.min = 50.0;
                threshold.max = 100.0;
                threshold.description = "Default threshold (error fallback)";
            }

            thresholds[measureName] = threshold;
        }

        cout << "Successfully loaded " << thresholds.size() << " thresholds" << endl;
    }
    catch (const exception &e)
    {
        cerr << "ERROR parsing thresholds file: " << e.what() << endl;
        throw;
    }

    return thresholds;
}

string measureEnumToString(QualityMeasure measure)
{
    try
    {
        return string(magic_enum::enum_name(measure));
    }
    catch (...)
    {
        return "";
    }
}

void printUsage(const string &executable)
{
    cout << "Usage: " << executable << " [options]" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "  -i, --image <path>        Path to input image (required)" << endl;
    cout << "  -c, --config-dir <path>   Directory containing OFIQ configuration" << endl;
    cout << "                            (default: current directory)" << endl;
    cout << "  -t, --thresholds <path>   Path to ICAO compliance thresholds file" << endl;
    cout << "                            (default: icao_compliance_config.json)" << endl;
    cout << "  -o, --output <path>       Path to output JSON file (default: stdout)" << endl;
    cout << "  -h, --help                Show this help message" << endl;
    cout << endl;
    cout << "Example:" << endl;
    cout << "  " << executable << " -i test.jpg -c data -t icao_compliance_config.json -o result.json" << endl;
}

json extractFacialLandmarks(const OFIQ::FaceLandmarks &landmarks)
{
    json result = json({});

    if (landmarks.type != LandmarkType::LM_98 || landmarks.landmarks.empty())
    {
        return result;
    }

    const auto &points = landmarks.landmarks;

    // ADNet 98-point landmark indices (from adnet_FaceMap.h)
    // Left eye (image left, person's right eye): indices 60-67
    // Right eye (image right, person's left eye): indices 68-75
    // Nose tip: index 54
    // Mouth outer: indices 76-87
    // Mouth inner: indices 88-95

    // Extract left eye points
    json leftEyeJson = json({});
    json leftEyePointsArray = json::array({});
    double leftEyeSumX = 0.0, leftEyeSumY = 0.0;
    int leftEyeCount = 0;

    for (int i = 60; i <= 67; i++)
    {
        if (i < points.size())
        {
            json pointJson = json({});
            pointJson["x"] = points[i].x;
            pointJson["y"] = points[i].y;
            leftEyePointsArray.push_back(pointJson);

            leftEyeSumX += points[i].x;
            leftEyeSumY += points[i].y;
            leftEyeCount++;
        }
    }

    if (leftEyeCount > 0)
    {
        leftEyeJson["points"] = leftEyePointsArray;
        json centerJson = json({});
        centerJson["x"] = leftEyeSumX / leftEyeCount;
        centerJson["y"] = leftEyeSumY / leftEyeCount;
        leftEyeJson["center"] = centerJson;
        result["left_eye"] = leftEyeJson;
    }

    // Extract right eye points
    json rightEyeJson = json({});
    json rightEyePointsArray = json::array({});
    double rightEyeSumX = 0.0, rightEyeSumY = 0.0;
    int rightEyeCount = 0;

    for (int i = 68; i <= 75; i++)
    {
        if (i < points.size())
        {
            json pointJson = json({});
            pointJson["x"] = points[i].x;
            pointJson["y"] = points[i].y;
            rightEyePointsArray.push_back(pointJson);

            rightEyeSumX += points[i].x;
            rightEyeSumY += points[i].y;
            rightEyeCount++;
        }
    }

    if (rightEyeCount > 0)
    {
        rightEyeJson["points"] = rightEyePointsArray;
        json centerJson = json({});
        centerJson["x"] = rightEyeSumX / rightEyeCount;
        centerJson["y"] = rightEyeSumY / rightEyeCount;
        rightEyeJson["center"] = centerJson;
        result["right_eye"] = rightEyeJson;
    }

    // Extract nose tip
    if (54 < points.size())
    {
        try
        {
            // Check if the point has valid coordinates
            if (!std::isnan(points[54].x) && !std::isnan(points[54].y) &&
                std::isfinite(points[54].x) && std::isfinite(points[54].y))
            {
                json noseJson = json({});
                noseJson["x"] = points[54].x;
                noseJson["y"] = points[54].y;
                result["nose_tip"] = noseJson;
            }
            else
            {
                cerr << "WARNING: Nose tip coordinates are not valid numbers: x="
                     << points[54].x << ", y=" << points[54].y << endl;
            }
        }
        catch (const exception &e)
        {
            cerr << "ERROR extracting nose tip: " << e.what() << endl;
        }
    }

    // Extract mouth points
    json mouthJson = json({});
    json mouthOuterPointsArray = json::array({});
    json mouthInnerPointsArray = json::array({});
    double mouthSumX = 0.0, mouthSumY = 0.0;
    int mouthCount = 0;

    // Mouth outer points (76-87)
    for (int i = 76; i <= 87; i++)
    {
        if (i < points.size())
        {
            json pointJson = json({});
            pointJson["x"] = points[i].x;
            pointJson["y"] = points[i].y;
            mouthOuterPointsArray.push_back(pointJson);

            mouthSumX += points[i].x;
            mouthSumY += points[i].y;
            mouthCount++;
        }
    }

    // Mouth inner points (88-95)
    for (int i = 88; i <= 95; i++)
    {
        if (i < points.size())
        {
            json pointJson = json({});
            pointJson["x"] = points[i].x;
            pointJson["y"] = points[i].y;
            mouthInnerPointsArray.push_back(pointJson);

            mouthSumX += points[i].x;
            mouthSumY += points[i].y;
            mouthCount++;
        }
    }

    if (mouthCount > 0)
    {
        mouthJson["outer_points"] = mouthOuterPointsArray;
        mouthJson["inner_points"] = mouthInnerPointsArray;

        json centerJson = json({});
        centerJson["x"] = mouthSumX / mouthCount;
        centerJson["y"] = mouthSumY / mouthCount;
        mouthJson["center"] = centerJson;

        result["mouth"] = mouthJson;
        result["mouth_center"] = centerJson; // Also include mouth center separately for easy access
    }

    // Add landmark count and type
    result["landmark_count"] = points.size();
    result["landmark_type"] = "LM_98";

    return result;
}
