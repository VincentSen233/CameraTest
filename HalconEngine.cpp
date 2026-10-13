#include "HalconEngine.h"
#include <QFileInfo>

using namespace HalconCpp;
using namespace HDevEngineCpp;

HalconEngine::HalconEngine()
{
    // 这里刻意什么都不做：不自动加载脚本、不自动读图
}

HalconEngine::~HalconEngine()
{
}

// 打开 JIT 编译（即时编译），让脚本跑得接近 C++ 的速度
// 不开也能跑，只是慢一些
void HalconEngine::enableJit()
{
    HDevEngine().SetEngineAttribute("execute_procedures_jit_compiled", "true");
}

// ======================================================================
// 按文件后缀自动分流：hdpl / hdvp 走过程库，hdev 走程序
// ======================================================================
bool HalconEngine::loadScript(const QString& path, const QString& procName, QString* err)
{
    const QString suffix = QFileInfo(path).suffix().toLower();

    if (suffix == "hdpl" || suffix == "hdvp")
        return loadProcedureLibrary(path, procName, err);

    if (suffix == "hdev")
        return loadProgramFile(path, procName, err);

    if (err)
        *err = QString("不认识的文件类型：%1（目前只认 .hdpl / .hdev / .hdvp）").arg(suffix);
    return false;
}

// ======================================================================
// 方式 A：载入 .hdpl 过程库
// ======================================================================
bool HalconEngine::loadProcedureLibrary(const QString& path, const QString& procName, QString* err)
{
    try
    {
        // 告诉引擎过程库在哪。注意要写到"文件本身"，只写到目录是不行的。
        // （如果传进来的本来就是目录，Halcon 会去扫目录里的 .hdvp，两种都能用）
        HDevEngine().SetProcedurePath(path.toLocal8Bit().constData());

        enableJit();

        // 按名字把过程取出来，取得到才说明库真的加载成功了
        m_proc = HDevProcedure(procName.toLocal8Bit().constData());
        if (!m_proc.IsLoaded())
        {
            if (err)
                *err = QString("过程库里找不到名为 \"%1\" 的过程").arg(procName);
            return false;
        }

        // 预热编译，免得第一次跑的时候突然卡一下
        m_proc.CompileUsedProcedures();

        m_kind       = ScriptKind::ProcedureLibrary;
        m_scriptPath = path;
        m_procName   = procName;
        m_ready      = true;
        return true;
    }
    catch (HDevEngineException& e)   // 引擎错误：脚本没找到、参数名不对等
    {
        if (err) *err = QString::fromLocal8Bit(e.Message());
        return false;
    }
    catch (HException& e)            // Halcon 算子本身的错误
    {
        if (err) *err = QString::fromLocal8Bit(e.ErrorMessage().Text());
        return false;
    }
}

// ======================================================================
// 方式 B：载入 .hdev 程序文件
// ======================================================================
bool HalconEngine::loadProgramFile(const QString& path, const QString& procName, QString* err)
{
    try
    {
        enableJit();

        // hdev 要先整体加载成"程序"，再从里面挑一个局部过程来用
        m_program = HDevProgram(path.toLocal8Bit().constData());
        if (!m_program.IsLoaded())
        {
            if (err)
                *err = QString("hdev 加载失败：%1").arg(path);
            return false;
        }

        // 第二个参数是 program 对象本身，不是文件名 —— 这是和 hdpl 最大的区别
        m_proc = HDevProcedure(m_program, procName.toLocal8Bit().constData());
        if (!m_proc.IsLoaded())
        {
            if (err)
                *err = QString("程序里找不到名为 \"%1\" 的局部过程").arg(procName);
            return false;
        }

        m_proc.CompileUsedProcedures();

        m_kind       = ScriptKind::Program;
        m_scriptPath = path;
        m_procName   = procName;
        m_ready      = true;
        return true;
    }
    catch (HDevEngineException& e)
    {
        if (err) *err = QString::fromLocal8Bit(e.Message());
        return false;
    }
    catch (HException& e)
    {
        if (err) *err = QString::fromLocal8Bit(e.ErrorMessage().Text());
        return false;
    }
}

// ======================================================================
// 读离线图片
// ======================================================================
bool HalconEngine::loadImage(const QString& imagePath, QString* err)
{
    try
    {
        // HImage 的构造函数传文件名，Halcon 自动按后缀解码
        m_image = HImage(imagePath.toLocal8Bit().constData());
        return true;
    }
    catch (HException& e)
    {
        if (err) *err = QString::fromLocal8Bit(e.ErrorMessage().Text());
        return false;
    }
}

// ======================================================================
// 执行算法
// ======================================================================
bool HalconEngine::run(HRegion* outRegion, bool* outOk, double* outValue, QString* err)
{
    if (!m_ready)
    {
        if (err) *err = "还没加载脚本";
        return false;
    }
    if (!m_image.IsInitialized())
    {
        if (err) *err = "还没载入图片";
        return false;
    }

    try
    {
        // 每次调用新建一个"传话筒"：塞参数 → Execute → 取结果
        // 不要多个线程共用一个 HDevProcedureCall
        HDevProcedureCall call(m_proc);

        // 参数名必须和脚本里定义的完全一致，包括大小写
        call.SetInputIconicParamObject("Image",     m_image);
        call.SetInputCtrlParamTuple  ("Threshold", HTuple(128));

        call.Execute();

        // .I() 取 int，.D() 取 double，.L() 取长整数
        if (outRegion) *outRegion = HRegion(call.GetOutputIconicParamObject("Region"));
        if (outOk)     *outOk     = (call.GetOutputCtrlParamTuple("IsOK").I() == 1);
        if (outValue)  *outValue  =  call.GetOutputCtrlParamTuple("Area").D();

        return true;
    }
    catch (HDevEngineException& e)
    {
        if (err) *err = QString::fromLocal8Bit(e.Message());
        return false;
    }
    catch (HException& e)
    {
        if (err) *err = QString::fromLocal8Bit(e.ErrorMessage().Text());
        return false;
    }
}

// ======================================================================
// Halcon 图像 → QImage
// ======================================================================
QImage HalconEngine::toQImage(const HImage& img)
{
    if (!img.IsInitialized())
        return QImage();

    HImage gray = img;

    // 彩图先转灰（3 通道 → 1 通道）
    if (gray.CountChannels().I() == 3)
        gray = gray.Rgb1ToGray();

    // 统一转成 8 位灰度，这样后面只需要处理一种情况
    gray = gray.ConvertImageType("byte");

    // 取图像数据指针（这块内存归 Halcon 管，随时可能被回收）
    HString  type;
    Hlong    w = 0, h = 0;
    void*    p = gray.GetImagePointer1(&type, &w, &h);

    // 末尾的 .copy() 不能省：不拷一份的话 Halcon 回收内存后就成野指针了
    return QImage((const uchar*)p, (int)w, (int)h, (int)w,
                  QImage::Format_Grayscale8).copy();
}
