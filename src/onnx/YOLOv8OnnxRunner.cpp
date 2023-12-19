// #include "Configuration.h"
#include "YOLOv8OnnxRunner.h"

YOLOv8OnnxRunner::YOLOv8OnnxRunner(Configuration cfg) : 
    num_threads(cfg.num_thread)
{
    try
    {
        this->confThreshold = cfg.confThreshold;
        this->nmsThreshold = cfg.nmsThreshold;
        #ifdef USE_CUDA
            // GPU FP32 inference
            cfg.cudaEnable = true
        # elif
            // CPU inference
            cfg.cudaEnable = false
        #endif
            InitOrtEnv(cfg);
    }
    catch(const std::exception& e)
    {
        std::cerr << "[ERROR] : " << e.what() << '\n';
    }
    
}

YOLOv8OnnxRunner::~YOLOv8OnnxRunner()
{
    delete session;
}

void YOLOv8OnnxRunner::setConfThreshold(float threshold)
{
    this->confThreshold = threshold;
}

void YOLOv8OnnxRunner::setNMSThreshold(float threshold)
{
    this->nmsThreshold = threshold;
}

void YOLOv8OnnxRunner::softmax()
{

}

void YOLOv8OnnxRunner::InitOrtEnv(Configuration cfg)
{
    env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "YOLOv8Model");
    if (cfg.cudaEnable)
    {
        OrtCUDAProviderOptions cudaOption;
        cuda_options.device_id = 0;
        session_options.AppendExecutionProvider_CUDA(cudaOption);
    }

	session_options = Ort::SessionOptions();
	session_options.SetInterOpNumThreads(cfg.num_thread);
	session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    std::wstring model_path = std::wstring(cfg.ModelPath.begin() , cfg.ModelPath.end());
    session = new Ort::Session(env , model_path.c_str() , session_options);

    Ort::AllocatorWithDefaultOptions allocator;
    size_t inputNodesNum = session->GetInputCount();
    for (size_t i = 0; i < inputNodesNum; i++)
    {
        Ort::AllocatedStringPtr input_node_name = session->GetInputNameAllocated(i, allocator);
        char* temp_buf = new char[50];
        strcpy(temp_buf, input_node_name.get());
        inputNodeNames.push_back(temp_buf);
    }
    
    size_t OutputNodesNum = session->GetOutputCount();
    for (size_t i = 0; i < OutputNodesNum; i++)
    {
        Ort::AllocatedStringPtr output_node_name = session->GetOutputNameAllocated(i, allocator);
        char* temp_buf = new char[10];
        strcpy(temp_buf, output_node_name.get());
        outputNodeNames.push_back(temp_buf);
    }

}

cv::Mat YOLOv8OnnxRunner::Preprocess(cv::Mat srcImage , int *new_h , int *new_w , int *pad_w , int *pad_h)
{   
    int src_width = srcImage.cols , src_height = srcImage.rows;
    *new_w = this->input_width;
    *new_h = this->input_height;
    cv::Mat resizeImage;

    if (src_height != src_width)
    {
        float hw_scale = (float) src_height / src_width;
        if (hw_scale > 1)
        {
            *new_h = this->input_height;
            *new_w = int(this->input_width / hw_scale);
            cv::resize(srcImage , resizeImage , cv::Size(*new_w , *new_h) , cv::INTER_AREA);
            *pad_w = int((this->input_width - *new_w) / 2);
            cv::copyMakeBorder(resizeImage , resizeImage , 0 , 0 , *pad_w , *pad_w , cv::BorderTypes::BORDER_CONSTANT, cv::Scalar(0, 0, 0))
        } else 
        {
            *new_h = int(this->input_height * hw_scale);
            *new_w = this->input_width;
            cv::resize(srcImage , resizeImage , cv::Size(*new_w , *new_h) , cv::INTER_AREA);
            *pad_h = int((this->input_height - *new_h) / 2);
            cv::copyMakeBorder(resizeImage , resizeImage , *pad_h , *pad_h , 0 , 0 , cv::BorderTypes::BORDER_CONSTANT, cv::Scalar(0, 0, 0))
        }
    } else
    {
        cv::resize(srcImage , resizeImage , cv::Size(*new_w , *new_h) , cv::INTER_AREA);
    }

    return resizeImage;
}

cv::Mat YOLOv8OnnxRunner::Proprocess()
{
    return cv::Mat();
}

cv::Mat YOLOv8OnnxRunner::InferenceSingleImage(const cv::Mat& srcImage)
{
    int new_w = 0 , new_h = 0 , pad_w = 0 , pad_h = 0;
    
    cv::Mat resizeImage = Preprocess(srcImage , &new_w , &new_h , &pad_w , &pad_h);
    
    cv::Mat resultImage = Proprocess();

    return resultImage;
}

void YOLOv8OnnxRunner::VisualizationPredicition()
{

}