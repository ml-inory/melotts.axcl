#include "onnxruntime_cxx_api.h"

class OnnxWrapper {
public:
    OnnxWrapper():
        m_session(nullptr) {

    }
    ~OnnxWrapper() {
        Release();
    }

    int Init(const std::string& model_file);

    std::vector<Ort::Value> Run(std::vector<int>& phone, 
                                std::vector<int>& tones,
                                std::vector<int>& langids,
                                std::vector<float>& g,
                                
                                float noise_scale,
                                float length_scale,
                                float noise_scale_w,
                                float sdp_ratio);

    inline int GetInputSize(int index) const {
        return m_input_sizes[index];
    }

    inline int GetOutputSize(int index) const {
        return m_output_sizes[index];
    }

    int Release() {
        if (m_session) {
            delete m_session;
            m_session = nullptr;
        }
        return 0;
    }

private:
    Ort::Env m_ort_env;
    Ort::Session* m_session;
    int m_input_num, m_output_num;
    std::vector<std::string> m_input_names, m_output_names;
    std::vector<int> m_input_sizes, m_output_sizes;
};