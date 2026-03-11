// ---------------------------------------------------------------------------
// Sword3 Engine (c) 1999-2000 by Kingsoft
//
// Mục đích: Khai báo lớp KUpdateLayer điều phối cập nhật tài nguyên/phiên bản
//           của trò chơi, quản lý tải xuống, hiển thị tiến trình và chuyển cảnh.
// Tác giả gốc: WangWei (Daphnis)
// Ngày gốc:    2000.08.08
// ---------------------------------------------------------------------------
#pragma once

// Chuẩn C++
#include <cstddef>
#include <cstdint>
#include <string>

// Phụ thuộc engine/cocos
#include "cocos2d.h"
#include "engine/KPakList.h"
#include "engine/KDownLoadFile.h"
USING_NS_AX;

#include "AudioEngine.h"
#include "platform/FileUtils.h"
#include "../network/ClientStage.h"
#include "../network/Thread.h"
#include "LoginDef.h"
#include "assets-manager/AssetsManager.h"

// Dữ liệu danh sách gói (pak) cần cập nhật.
// Giữ nguyên kích thước/trật tự trường để tương thích pool/packet chain.
struct Kpaklistdata {
    char pakName[128];   // Tên gói pak
    int  _pakreload;     // Cờ yêu cầu nạp lại pak
    bool _isbreak;       // Cờ dừng/gián đoạn
};

class KUpdateLayer : public Layer,
public ax::extension::AssetsManagerDelegateProtocol,
protected CThread {
public:
    KUpdateLayer();
    ~KUpdateLayer();
    static Scene* scene() {
        Scene* scene = Scene::create();
        scene->addChild(KUpdateLayer::create());
        return scene;
    }
    CREATE_FUNC(KUpdateLayer);
    using CThread::Start;
    void reset(Ref* pSender);             // Đặt lại phiên bản
    void getClientVersion(Ref* pSender);  // Lấy phiên bản client hiện tại
    void checkUpdate(Ref* pSender);       // Kiểm tra có bản cập nhật hay không
    void updates(Ref* pSender);           // Thực thi cập nhật
    void enterScene(Ref* pSender);        // Vào cảnh game sau khi cập nhật
    
    bool init() override;
    
    // Vòng lặp cập nhật khung hình
    void update(float delta) override;
    
    // Thân hàm luồng nền
    void Run() override;
    
protected:
    // Khởi tạo thư mục tải xuống
    void initDownloadDir();
    
    // Tạo thư mục tạm tải xuống theo đường dẫn
    void creatDownloadDir(std::string nTempPath);
    
    // Xóa thư mục (đệ quy nếu cần)
    void deleteDir(std::string dir);
    
private:
    // Con trỏ tải tệp (giữ nguyên tên/kiểu để tương thích API khác)
    downloadFile* downPtr {nullptr};
    
    // UI thanh tiến trình
    Sprite*         m_pLoadBarStart {nullptr};   // Ảnh bắt đầu tải
    ProgressTimer*  m_pLoadBarEnd   {nullptr};   // Thanh tiến trình kết thúc
    
    // Thống kê tiến trình
    int   m_nAllLoadResources   {0};  // Tổng số tài nguyên cần tải
    int   m_nLoadedResources    {0};  // Số tài nguyên đã tải
    int   m_nLoadedFinish       {0};  // Số tài nguyên tải xong
    float m_fProgressIndex      {0};  // Giá trị tiến trình 0..100
    
    // Nhãn hiển thị
    Label* m_pLabelLoading {nullptr}; // Nhãn "Đang tải"
    Label* m_pLabelPercent {nullptr}; // Nhãn phần trăm
    
    // Bắt đầu xóa/tinh chỉnh sau khi tải xong
    void startload();
    
    // Nhãn debug/thông báo
    Label* m_label {nullptr};
    
    // Đường dẫn tải xuống và ghi được
    std::string m_downloadDir;  // Thư mục tải xuống
    std::string m_WritablePath; // Thư mục có thể ghi
    
    // Tiện ích tệp
    void copyData(const char* pFileName);
    int  isFileExist(const char* pFileName);
    
    // Hoàn tất tải
    void loadSuccess();
    
    // Trạng thái cập nhật
    enum UpdataState {
        STATE_NONE_UP,         // Không làm gì
        STATE_RUN_UP,          // Đang xử lý cập nhật
        STATE_FLS_UP,          // Làm mới
        STATE_ON_FIEL_UP,      // Lỗi tệp
        STATE_ON_URLFIEL_UP,   // Lỗi URL
        STATE_ON_NET_UP,       // Lỗi mạng
        STATE_LOAD_LISTDATA,   // Đang tải danh sách dữ liệu
        STATE_DWON_LISTDATA    // Đang tải xuống danh sách dữ liệu
    };
    
    // Biến trạng thái/tạm
    std::size_t  writelen {0};            // Số byte đã ghi
    std::string  nCurpFileName;           // Tên tệp hiện tại
    int          nPakCount {0};           // Số lượng pak
    char         downUrlmask[64] = {0};   // Mặt nạ URL tải xuống
    int          nDownState {0};          // Trạng thái tải xuống
    Kpaklistdata* pListData {nullptr};    // Danh sách pak
    int          pakDownCount {0};        // Đếm số pak đã tải
    int          isOpenDown {0};          // Cờ mở tải
    unsigned int errorLoop {0};           // Vòng lặp lỗi
    bool         isHaveClient {false};    // Có client hay không
    bool         isOver {false};          // Đã xong hay chưa
    bool         m_IsInPutIpPadIsOpen {false}; // Trạng thái input IPPad mở
    
    // Bắt đầu nhập (callback UI)
    void startInPutCallback(Node* pNode);
};
