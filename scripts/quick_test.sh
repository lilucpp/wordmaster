#!/bin/bash

# ============================================
# WordMaster 快速测试脚本
# 用于测试真实数据导入
# ============================================

set -e

# 颜色输出
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
CLI_TOOL="${BUILD_DIR}/wordmaster_cli"
TEST_DB="${BUILD_DIR}/test_wordmaster.db"

print_info "WordMaster 快速测试"
print_info "===================="

# 检查是否已构建
if [ ! -f "$CLI_TOOL" ]; then
    print_warning "CLI 工具未构建，开始构建..."
    cd "$PROJECT_ROOT"
    ./scripts/build.sh
fi

# 检查数据文件路径
if [ $# -lt 1 ]; then
    print_error "用法: $0 <词库元数据JSON文件路径>"
    echo ""
    echo "示例:"
    echo "  $0 ~/Downloads/recommend_word.json"
    echo ""
    echo "确保以下文件在同一目录:"
    echo "  - recommend_word.json (元数据)"
    echo "  - CET4_T.json (CET-4 单词)"
    echo "  - CET6_T.json (CET-6 单词)"
    echo "  - ... (其他词库文件)"
    exit 1
fi

META_JSON="$1"

if [ ! -f "$META_JSON" ]; then
    print_error "文件不存在: $META_JSON"
    exit 1
fi

print_info "元数据文件: $META_JSON"

# 获取文件目录
DATA_DIR="$(dirname "$META_JSON")"
print_info "数据目录: $DATA_DIR"

# 检查必要的词库文件
print_info "检查词库文件..."
REQUIRED_FILES=(
    "CET4_T.json"
    "CET6_T.json"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$DATA_DIR/$file" ]; then
        print_info "  ✓ $file"
    else
        print_warning "  ✗ $file (缺失)"
    fi
done

# 清理旧数据库
if [ -f "$TEST_DB" ]; then
    print_warning "删除旧的测试数据库..."
    rm "$TEST_DB"
fi

cd "$BUILD_DIR"

# 测试 1: 导入词库
print_info ""
print_info "测试 1: 导入词库"
print_info "=================="
time "$CLI_TOOL" --database "$TEST_DB" --import "$META_JSON"

# 测试 2: 列出所有词库
print_info ""
print_info "测试 2: 列出所有词库"
print_info "===================="
"$CLI_TOOL" --database "$TEST_DB" --list

# 测试 3: 查看 CET-4 统计
print_info ""
print_info "测试 3: CET-4 统计"
print_info "=================="
"$CLI_TOOL" --database "$TEST_DB" --stats cet4

# 测试 4: 查看 CET-4 单词样本
print_info ""
print_info "测试 4: CET-4 单词样本"
print_info "====================="
"$CLI_TOOL" --database "$TEST_DB" --samples cet4

# 测试 5: 搜索单词
print_info ""
print_info "测试 5: 搜索单词 'test'"
print_info "======================="
"$CLI_TOOL" --database "$TEST_DB" --search test

# 测试 6: 激活词库
print_info ""
print_info "测试 6: 激活 CET-4 词库"
print_info "======================="
"$CLI_TOOL" --database "$TEST_DB" --activate cet4

# 验证数据完整性
print_info ""
print_info "验证数据完整性"
print_info "=============="

if command -v sqlite3 &> /dev/null; then
    print_info "词库统计:"
    sqlite3 "$TEST_DB" "
    SELECT 
        b.id as '词库ID',
        b.name as '名称',
        b.word_count as '预期数量',
        COUNT(w.id) as '实际数量',
        CASE 
            WHEN b.word_count = COUNT(w.id) THEN '✓'
            ELSE '✗'
        END as '状态'
    FROM books b
    LEFT JOIN words w ON b.id = w.book_id
    GROUP BY b.id
    ORDER BY b.id;
    "
else
    print_warning "未安装 sqlite3，跳过数据完整性验证"
fi

# 性能报告
print_info ""
print_info "性能报告"
print_info "========"
DB_SIZE=$(du -h "$TEST_DB" | cut -f1)
print_info "数据库大小: $DB_SIZE"

if command -v sqlite3 &> /dev/null; then
    TOTAL_BOOKS=$(sqlite3 "$TEST_DB" "SELECT COUNT(*) FROM books;")
    TOTAL_WORDS=$(sqlite3 "$TEST_DB" "SELECT COUNT(*) FROM words;")
    print_info "总词库数: $TOTAL_BOOKS"
    print_info "总单词数: $TOTAL_WORDS"
fi

print_info ""
print_info "测试完成！"
print_info "=========="
print_info "数据库位置: $TEST_DB"
print_info ""
print_info "后续操作:"
echo "  1. 查看更多词库: $CLI_TOOL --database $TEST_DB --list"
echo "  2. 搜索单词: $CLI_TOOL --database $TEST_DB --search <单词>"
echo "  3. 查看统计: $CLI_TOOL --database $TEST_DB --stats <词库ID>"
echo "  4. 使用 SQLite: sqlite3 $TEST_DB"