# WordMaster 快速开始指南

## 5分钟快速上手

### 第一步：准备环境

```bash
# 安装依赖（Ubuntu/Debian）
sudo apt update
sudo apt install -y build-essential cmake git
sudo apt install -y qtbase5-dev libgtest-dev
```

### 第二步：克隆并构建

```bash
# 克隆项目
git clone <repository-url>
cd WordMaster

# 赋予脚本执行权限
chmod +x scripts/*.sh

# 构建项目
make build

# 或使用脚本
./scripts/build.sh
```

**预期输出：**
```
[INFO] WordMaster Build Script
[INFO] ========================
[INFO] Build Type: Debug
[INFO] Configuring project...
[INFO] Building project...
[INFO] Running tests...
[INFO] All tests passed!
[INFO] Build completed successfully!
```

### 第三步：导入真实数据

**准备数据文件：**
```bash
# 下载或复制您的词库文件到某个目录
# 例如：~/Downloads/词库数据/
#   ├── 1764429285470_recommend_word.json
#   ├── CET4_T.json
#   ├── CET6_T.json
#   └── ... (其他词库文件)
```

**快速导入：**
```bash
# 方法1: 使用快速测试脚本（推荐）
./scripts/quick_test.sh ~/Downloads/词库数据/1764429285470_recommend_word.json

# 方法2: 使用 Makefile
make import DATA=~/Downloads/词库数据/1764429285470_recommend_word.json

# 方法3: 直接使用 CLI 工具
cd build
./wordmaster_cli --import ~/Downloads/词库数据/1764429285470_recommend_word.json
```

**预期输出：**
```
WordMaster CLI v1.0.0
数据库: wordmaster.db
================================================================================
开始导入词库...
元数据文件: /home/user/Downloads/词库数据/1764429285470_recommend_word.json

导入结果:
  状态: 成功
  消息: 成功导入 14 个词库，共 45139 个单词
  导入词库数: 14
  导入单词数: 45139
```

### 第四步：验证数据

```bash
# 方法1: 使用验证脚本
./scripts/verify_data.sh build/wordmaster.db

# 方法2: 使用 Makefile
make verify
```

**预期输出：**
```
WordMaster 数据验证
====================
数据库: build/wordmaster.db

1. 表结构验证
-------------
✓ 表 books 存在
✓ 表 words 存在
✓ 表 study_records 存在
✓ 表 review_schedule 存在
✓ 表 word_tags 存在
✓ 表 user_preferences 存在

2. 数据完整性验证
----------------
✓ 词库数量: 14
✓ 单词数量: 45139

3. 词库-单词关联验证
------------------
✓ CET-4: 2607/2607 单词
✓ CET-6: 2345/2345 单词
✓ 专四: 4025/4025 单词
...

验证结果
========
通过: 25 / 25
所有测试通过！
```

### 第五步：探索数据

```bash
# 查看所有词库
make list

# 查看 CET-4 统计
make stats BOOK=cet4

# 搜索单词
cd build
./wordmaster_cli --search test

# 查看单词样本
./wordmaster_cli --samples cet4
```

---

## 常用命令速查

### 构建相关

```bash
make build              # Debug 构建
make build-release      # Release 构建
make clean              # 清理
```

### 测试相关

```bash
make test               # 所有测试
make test-unit          # 单元测试
make test-integration   # 集成测试
```

### 数据导入

```bash
# 导入词库
make import DATA=<词库元数据JSON路径>

# 查看词库
make list

# 查看统计
make stats BOOK=<词库ID>

# 验证数据
make verify
```

### CLI 工具

```bash
cd build

# 导入
./wordmaster_cli --import <元数据JSON>

# 列表
./wordmaster_cli --list

# 统计
./wordmaster_cli --stats <词库ID>

# 样本
./wordmaster_cli --samples <词库ID>

# 搜索
./wordmaster_cli --search <单词>

# 激活
./wordmaster_cli --activate <词库ID>

# 删除
./wordmaster_cli --delete <词库ID>
```

---

## 完整示例流程

```bash
# 1. 安装依赖
sudo apt install -y build-essential cmake qtbase5-dev libgtest-dev

# 2. 克隆项目
git clone <repository-url>
cd WordMaster

# 3. 构建
chmod +x scripts/*.sh
make build

# 4. 导入 CET-4 数据（示例）
make import DATA=~/Downloads/1764429285470_recommend_word.json

# 5. 验证数据
make verify

# 6. 查看词库
make list

# 7. 查看 CET-4 详情
make stats BOOK=cet4

# 8. 查看单词样本
cd build
./wordmaster_cli --samples cet4

# 9. 搜索单词
./wordmaster_cli --search alcohol

# 10. 激活 CET-4（准备学习）
./wordmaster_cli --activate cet4
```

---

## 故障排查

### 问题1: 构建失败 - 找不到 Qt5

**症状：**
```
CMake Error: Could not find a package configuration file provided by "Qt5"
```

**解决：**
```bash
sudo apt install qtbase5-dev
```

### 问题2: 测试失败 - 找不到 gtest

**症状：**
```
Could not find GTest
```

**解决：**
```bash
sudo apt install libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib
```

### 问题3: 导入失败 - 找不到词库文件

**症状：**
```
Failed to open file: /path/to/CET4_T.json
```

**解决：**
确保所有词库 JSON 文件和元数据文件在同一目录：
```bash
ls -l ~/Downloads/词库数据/
# 应该看到：
# 1764429285470_recommend_word.json
# CET4_T.json
# CET6_T.json
# ...
```

### 问题4: 脚本无法执行

**症状：**
```
Permission denied
```

**解决：**
```bash
chmod +x scripts/*.sh
```

---

## 下一步

✅ **已完成：**
- 环境搭建
- 项目构建
- 数据导入
- 数据验证

🚀 **接下来：**
- 学习功能（P0）
- 复习系统（P0）
- GUI 界面开发

📚 **更多文档：**
- [README_DEV.md](README_DEV.md) - 开发文档
- [CLI_USAGE_GUIDE.md](CLI_USAGE_GUIDE.md) - CLI 详细使用
- [BUILD_AND_TEST.md](BUILD_AND_TEST.md) - 构建测试指南

---

## 获取帮助

```bash
# 查看 Makefile 所有命令
make help

# 查看 CLI 工具帮助
cd build
./wordmaster_cli --help

# 查看文档列表
make docs
```

**遇到问题？**
1. 查看错误日志
2. 运行 `make verify` 检查数据
3. 查看相关文档
4. 提交 Issue

Happy Learning! 🎓📚