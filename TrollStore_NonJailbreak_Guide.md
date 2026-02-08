# 非越狱 + TrollStore 完整技术方案（iOS 15.4 / iPhone 13 Pro）

> 目标：在**非越狱、无 Root**环境下，安全升级 TrollStore、实现去广告/功能增强、生成自定义 dylib、完成 IPA 本地编译/重打包/签名，并通过 TrollStore 安装与验证。

---

## 0. 前置说明与适配性

- **设备**：iPhone 13 Pro（A15）
- **系统**：iOS 15.4（可用 TrollStore 体系）
- **架构**：arm64e（A12+ 机型）
- **限制**：无越狱、无 Root、不可用 Substrate/Substitute 全局注入

> 该方案仅面向**合法、本人设备**与**已获取合法安装包**的应用调试/学习场景。

---

## 一、TrollStore 升级（安全、无数据丢失）

### 1) 兼容性判断
- **iOS 15.4**：支持 TrollStore 1.x 体系。
- **A15 / arm64e**：确保新版 TrollStore 已适配 arm64e。

### 2) 推荐升级路径（优先级）
1. **TrollStore 内置自更新**  
   - 最稳妥，不改动现有已安装 IPA 数据目录。  
2. **覆盖安装（Reinstall）**  
   - 使用最新 TrollStore IPA 直接覆盖安装。  
3. **替换/修复安装**  
   - 仅在 TrollStore 自更新不可用时使用。

### 3) 升级影响说明
- **已安装 IPA**：不丢失，但建议升级前备份关键应用数据。
- **权限（com.apple.private）**：依然基于 TrollStore 的特权签名机制，不会被抹除。
- **dylib 注入方案**：已注入的 IPA 仍可运行，但**建议重签/重打包**以保证兼容性。

---

## 二、去广告方案设计（非越狱 TrollStore）

### 1) 可行方案对比
| 方案 | 可行性 | 优点 | 限制 |
|---|---|---|---|
| **dylib Hook（NSURLSession/WKWebView/SDK）** | ✅ | 最强、精准拦截 | 需注入、适配目标 App |
| **App 内方法拦截** | ✅ | 可对业务逻辑精准改写 | 需逆向分析 |
| **Hosts / DNS** | ⚠️ | 操作简单 | 非系统级，受限于 App 自定义 DNS/HTTPS |

### 2) 与越狱环境差异
| 项目 | 越狱（Substrate） | TrollStore |
|---|---|---|
| 全局注入 | ✅ | ❌ |
| 单 App 注入 | ✅ | ✅ |
| 系统级 Hosts/代理 | ✅ | ❌ |

### 3) 推荐方案
- **优先选择 dylib Hook + App 内方法拦截组合**：  
  - 通过 Hook 网络层或广告 SDK 提前拦截。  
  - 对业务层兜底修正（隐藏 View、阻断弹窗等）。  

---

## 三、dylib 生成（Theos）

### 1) Theos 初始化
```bash
git clone https://github.com/theos/theos.git ~/theos
export THEOS=~/theos
```

### 2) 创建 dylib 项目
```bash
$THEOS/bin/nic.pl
# 选择 "iphone/library" (dylib)
```

### 3) 示例 Hook（Logos / Objective-C）
```objc
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

%hook NSURLSession
- (NSURLSessionDataTask *)dataTaskWithRequest:(NSURLRequest *)request
                            completionHandler:(void (^)(NSData *data, NSURLResponse *response, NSError *error))completionHandler {
    NSURL *url = request.URL;
    if ([url.absoluteString containsString:@"ads"] || [url.host containsString:@"ad"]) {
        NSLog(@"[NoAds] Blocked: %@", url.absoluteString);
        if (completionHandler) {
            completionHandler([NSData data], nil, nil);
        }
        return nil;
    }
    return %orig;
}
%end
```

### 4) 编译要点
- **架构**：arm64 + arm64e（A12+）  
- **THEOS\_DEVICE\_IP**：仅在真机调试时使用  
- **编译参数**：
  - `ARCHS=arm64 arm64e`
  - `TARGET=iphone:clang:latest:15.0`

---

## 四、本地编译与 IPA 重打包

### 1) macOS / Linux 编译流程
1. 准备目标 App IPA（合法来源）  
2. 解包 IPA：  
   ```bash
   unzip Target.ipa -d PayloadDir
   ```
3. 将编译好的 dylib 放入：  
   ```
   PayloadDir/Payload/Target.app/Frameworks/
   ```

### 2) 注入方式（非全局）
使用 `insert_dylib` 或 `optool` 修改可执行文件的 Load Command：  
```bash
insert_dylib \
  --strip-codesig \
  --all-yes \
  @executable_path/Frameworks/YourHook.dylib \
  PayloadDir/Payload/Target.app/Target
```

### 3) 签名与重打包
```bash
ldid -S PayloadDir/Payload/Target.app/Target
ldid -S PayloadDir/Payload/Target.app/Frameworks/YourHook.dylib
cd PayloadDir && zip -r Repacked.ipa Payload
```

> TrollStore 支持安装已签名 IPA（自签 + 特权签名机制）。

---

## 五、安装与验证

### 1) 安装
将 Repacked.ipa 通过 TrollStore 安装。

### 2) 验证 dylib 是否加载
常用验证方式：  
- 在 Hook 中输出日志（Console / syslog）
- 增加明显的 UI 修改（如弹窗提示）

### 3) 常见失败场景与排查
| 场景 | 可能原因 | 解决方案 |
|---|---|---|
| 闪退 | 架构不匹配 | 确保 arm64e 编译 |
| 去广告无效 | Hook 点不正确 | 使用 class-dump / Hopper 再定位 |
| 签名错误 | codesign/ldid 问题 | 重新签名 |

---

## 六、风险控制与回滚

### 1) 可逆操作
- **重装原版 IPA**：即可恢复无注入状态。  
- **移除 dylib**：重新解包并删除 Frameworks 内 dylib。  

### 2) iOS 版本限制
- iOS 15.0-15.4 兼容性较好  
- 15.5+ 可能需要特定 TrollStore 支持  

---

## 总结建议

- **升级 TrollStore**：优先使用内置更新  
- **去广告方案**：dylib Hook + 业务方法拦截  
- **注入策略**：单 App 注入，避免全局改动  
- **回滚**：随时保留原始 IPA 以便恢复  

