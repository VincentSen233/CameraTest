#pragma once
#pragma execution_character_set("utf-8")

/********************************************************************
 * HalconEngine —— 把 Halcon 相关的所有操作都关在这个类里面
 *
 * 用法约定（重要）：
 *   1. 这个类不会自动加载任何 .hdpl / .hdev 脚本，
 *      必须由用户手动选择文件后调用 loadScript() 才会加载。
 *   2. 图像处理结果通过 run() 的输出参数取出，
 *      脚本里不要写 dev_display 之类的显示算子（HDevEngine 里是空实现，写了不生效）。
 *   3. 所有函数都不抛异常，失败时返回 false 并把原因写进 errMsg。
 ********************************************************************/

#include "HalconCpp.h"      // 图像处理核心：HImage / HRegion
#include "HDevEngineCpp.h"  // 脚本引擎：加载 hdpl / hdev
#include <QString>
#include <QImage>

// 当前脚本的类型，方便外面查询和区分
enum class ScriptKind
{
    None,               // 还没加载
    ProcedureLibrary,   // .hdpl 过程库（也兼容放着 .hdvp 的目录）
    Program             // .hdev 程序文件
};

class HalconEngine
{
public:
    HalconEngine();
    ~HalconEngine();

    // ------------------------------------------------------------------
    // 载入脚本
    // ------------------------------------------------------------------

    // 按文件后缀自动分流：.hdpl 走过程库那条路，.hdev 走程序那条路
    // procName = 要调用的过程名，比如 "do_inspect"
    bool loadScript(const QString& path, const QString& procName,
                    QString* errMsg = nullptr);

    // ------------------------------------------------------------------
    // 读离线图片
    // ------------------------------------------------------------------

    // 从硬盘读一张图，png / bmp / tif / jpg 都行，Halcon 自己按后缀解码
    bool loadImage(const QString& imagePath, QString* errMsg = nullptr);

    // ------------------------------------------------------------------
    // 执行算法
    // ------------------------------------------------------------------

    // 拿当前这张图，跑当前这个脚本
    // 三个输出参数都可以传 nullptr，表示不关心该项
    bool run(HalconCpp::HRegion* outRegion,   // 输出：找到的区域
             bool*               outOk,       // 输出：true=合格
             double*             outValue,    // 输出：面积之类的数值
             QString*            errMsg = nullptr);

    // ------------------------------------------------------------------
    // 取数据 / 显示辅助
    // ------------------------------------------------------------------

    HalconCpp::HImage image()      const { return m_image; }       // 当前这张图
    bool              isReady()    const { return m_ready; }       // 脚本加载好了没
    ScriptKind        kind()       const { return m_kind; }
    QString           scriptPath() const { return m_scriptPath; }  // 当前脚本路径
    QString           procName()   const { return m_procName; }    // 当前过程名

    // 静态工具：Halcon 图像 → QImage，方便丢给 QLabel 显示
    static QImage toQImage(const HalconCpp::HImage& img);

private:
    bool loadProcedureLibrary(const QString& path, const QString& procName, QString* err);
    bool loadProgramFile(const QString& path, const QString& procName, QString* err);
    void enableJit();

private:
    ScriptKind  m_kind = ScriptKind::None;
    QString     m_scriptPath;
    QString     m_procName;

    // 【注意】HDevProgram 必须是成员变量，不能写成局部变量。
    // 因为 HDevProcedure 是从 program 身上取出来的，
    // program 一旦销毁，取出的过程也就跟着失效了（hdev 模式最容易踩的坑）。
    HDevEngineCpp::HDevProgram    m_program;
    HDevEngineCpp::HDevProcedure  m_proc;

    HalconCpp::HImage  m_image;
    bool               m_ready = false;
};
