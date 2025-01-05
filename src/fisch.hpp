#pragma once

class Fisch
{
public:
    ~Fisch() = default;
    Fisch(const Fisch &) = delete;
    Fisch(Fisch &&) = delete;
    Fisch &operator=(const Fisch &) = delete;
    Fisch &operator=(Fisch &&) = delete;
    void startMacro();

    inline static Fisch &get()
    {
        static Fisch instance;
        return instance;
    }
    inline const HWND &getRobloxHWnd() const { return robloxHWnd_; }
    inline const RECT &getRobloxClientRect() const { return robloxClientRect_; }
    inline const POINT &getRobloxClientToWindowPoint() const { return robloxClientToWindowPoint_; }

    inline void updateRobloxClientRectAndRobloxClientToWindowPoint()
    {
        GetClientRect(robloxHWnd_, &robloxClientRect_);
        robloxClientToWindowPoint_ = {};
        ClientToScreen(robloxHWnd_, &robloxClientToWindowPoint_);
    }

    inline std::vector<std::tuple<int, int, int, int, int>> detectObjects()
    {
        static bool e{};
        static cv::dnn::Net net = cv::dnn::readNetFromONNX(executablePath_.concat("\\model.onnx").string());
        if (!e)
        {
            e = true;
            net.setPreferableBackend(cv::dnn::Backend::DNN_BACKEND_OPENCV);
            net.setPreferableTarget(cv::dnn::Target::DNN_TARGET_OPENCL);
        }

        cv::Mat mat = screenshot_();
        cv::Mat blob;
        std::vector<cv::Mat> outs;
        std::vector<std::tuple<int, int, int, int, float, int>> detectedObjects;
        constexpr float OBJECTNESS_THRESHOLD = 0.7f;
        constexpr float CLASS_CONFIDENCE_THRESHOLD = 0.7f;
        constexpr float NMS_THRESHOLD = 0.3f;

        cv::cvtColor(mat, mat, cv::COLOR_RGBA2RGB);
        cv::dnn::blobFromImage(mat, blob, 1 / 255.0, cv::Size(640, 640), cv::Scalar(), true, false);
        net.setInput(blob);
        net.forward(outs, net.getUnconnectedOutLayersNames());

        for (const auto &out : outs)
        {
            const float *data = out.ptr<float>();
            for (int i = 0; i < out.size[1]; i++)
            {
                if (data[i * out.size[2] + 4] < OBJECTNESS_THRESHOLD)
                    continue;

                float classConfidence{};
                int classId;
                for (int j = 5; j < out.size[2]; j++)
                {
                    if (data[i * out.size[2] + j] < classConfidence)
                        continue;
                    classConfidence = data[i * out.size[2] + j];
                    classId = j - 5;
                }

                if (classConfidence < CLASS_CONFIDENCE_THRESHOLD)
                    continue;

                float x = data[i * out.size[2] + 0];
                float y = data[i * out.size[2] + 1];
                float w = data[i * out.size[2] + 2];
                float h = data[i * out.size[2] + 3];
                int xScaled = static_cast<int>(((x - (w / 2)) / 640) * mat.cols);
                int yScaled = static_cast<int>(((y - (h / 2)) / 640) * mat.rows);
                int wScaled = static_cast<int>((w / 640) * mat.cols);
                int hScaled = static_cast<int>((h / 640) * mat.rows);
                detectedObjects.emplace_back(xScaled, yScaled, wScaled, hScaled, classConfidence, classId);
            }
        }

        std::vector<cv::Rect> boundingBoxes;
        std::vector<float> classConfidences;
        boundingBoxes.reserve(detectedObjects.size());
        classConfidences.reserve(detectedObjects.size());
        for (const auto &detectedObject : detectedObjects)
        {
            auto [x, y, w, h, classConfidence, _] = detectedObject;
            boundingBoxes.emplace_back(x, y, w, h);
            classConfidences.emplace_back(classConfidence);
        }

        std::vector<int> indices;
        cv::dnn::NMSBoxes(boundingBoxes, classConfidences, CLASS_CONFIDENCE_THRESHOLD, NMS_THRESHOLD, indices);

        std::vector<std::tuple<int, int, int, int, int>> detectedObjectsFiltered;
        detectedObjectsFiltered.reserve(indices.size());
        for (int index : indices)
        {
            auto [x, y, w, h, _, classId] = detectedObjects[index];
            detectedObjectsFiltered.emplace_back(x, y, w, h, classId);
        }

        return detectedObjectsFiltered;
    }

private:
    std::filesystem::path executablePath_{};
    HWND robloxHWnd_{};
    RECT robloxClientRect_{};
    POINT robloxClientToWindowPoint_{};

    enum classIds : int
    {
        fishingBar,
        shakeButton,
        fishingLine
    };

    Fisch();

    inline cv::Mat screenshot_()
    {
        HDC hDC = GetDC(robloxHWnd_);
        HDC hMemDC = CreateCompatibleDC(hDC);
        HBITMAP hBitmap = CreateCompatibleBitmap(hDC, robloxClientRect_.right, robloxClientRect_.bottom);
        cv::Mat mat(robloxClientRect_.bottom, robloxClientRect_.right, CV_8UC4);
        static BITMAPINFOHEADER bih{
            .biSize = sizeof(BITMAPINFOHEADER),
            .biWidth = robloxClientRect_.right,
            .biHeight = -robloxClientRect_.bottom,
            .biPlanes = 1,
            .biBitCount = 32,
            .biCompression = BI_RGB,
            .biSizeImage = 0,
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed = 0,
            .biClrImportant = 0};
        SelectObject(hMemDC, hBitmap);
        BitBlt(hMemDC, 0, 0, robloxClientRect_.right, robloxClientRect_.bottom, hDC, 0, 0, SRCCOPY);
        GetDIBits(hDC, hBitmap, 0, robloxClientRect_.bottom, mat.data, reinterpret_cast<BITMAPINFO *>(&bih), DIB_RGB_COLORS);
        DeleteObject(hBitmap);
        DeleteDC(hMemDC);
        ReleaseDC(robloxHWnd_, hDC);
        return mat;
    }
};
