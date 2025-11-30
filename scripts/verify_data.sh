#!/bin/bash

# ============================================
# WordMaster 数据验证脚本
# 验证导入的数据完整性和正确性
# ============================================

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_pass() {
    echo -e "${GREEN}✓${NC} $1"
}

print_fail() {
    echo -e "${RED}✗${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}!${NC} $1"
}

if [ $# -lt 1 ]; then
    echo "用法: $0 <数据库文件路径>"
    echo "示例: $0 build/wordmaster.db"
    exit 1
fi

DB_FILE="$1"

if [ ! -f "$DB_FILE" ]; then
    print_fail "数据库文件不存在: $DB_FILE"
    exit 1
fi

if ! command -v sqlite3 &> /dev/null; then
    print_fail "未安装 sqlite3"
    exit 1
fi

echo "WordMaster 数据验证"
echo "===================="
echo "数据库: $DB_FILE"
echo ""

# 测试计数器
TOTAL_TESTS=0
PASSED_TESTS=0

run_test() {
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    local test_name="$1"
    local test_query="$2"
    local expected="$3"
    
    result=$(sqlite3 "$DB_FILE" "$test_query")
    
    if [ "$result" = "$expected" ]; then
        print_pass "$test_name: $result"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        print_fail "$test_name: 期望 $expected, 实际 $result"
        return 1
    fi
}

# 1. 表结构验证
echo "1. 表结构验证"
echo "-------------"

TABLES=("books" "words" "study_records" "review_schedule" "word_tags" "user_preferences")

for table in "${TABLES[@]}"; do
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if sqlite3 "$DB_FILE" "SELECT name FROM sqlite_master WHERE type='table' AND name='$table';" | grep -q "$table"; then
        print_pass "表 $table 存在"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_fail "表 $table 不存在"
    fi
done

echo ""

# 2. 数据完整性验证
echo "2. 数据完整性验证"
echo "----------------"

# 检查词库数量
BOOK_COUNT=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM books;")
if [ "$BOOK_COUNT" -gt 0 ]; then
    print_pass "词库数量: $BOOK_COUNT"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_fail "没有词库数据"
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))

# 检查单词数量
WORD_COUNT=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM words;")
if [ "$WORD_COUNT" -gt 0 ]; then
    print_pass "单词数量: $WORD_COUNT"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_fail "没有单词数据"
fi
TOTAL_TESTS=$((TOTAL_TESTS + 1))

echo ""

# 3. 词库-单词关联验证
echo "3. 词库-单词关联验证"
echo "------------------"

sqlite3 "$DB_FILE" "
SELECT 
    b.id,
    b.name,
    b.word_count as expected,
    COUNT(w.id) as actual,
    CASE 
        WHEN b.word_count = COUNT(w.id) THEN 'PASS'
        ELSE 'FAIL'
    END as status
FROM books b
LEFT JOIN words w ON b.id = w.book_id
GROUP BY b.id
ORDER BY b.id;
" | while IFS='|' read -r id name expected actual status; do
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if [ "$status" = "PASS" ]; then
        print_pass "$name: $actual/$expected 单词"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_fail "$name: $actual/$expected 单词 (不匹配)"
    fi
done

echo ""

# 4. 数据质量检查
echo "4. 数据质量检查"
echo "--------------"

# 检查空单词
EMPTY_WORDS=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM words WHERE word IS NULL OR word = '';")
TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ "$EMPTY_WORDS" -eq 0 ]; then
    print_pass "无空单词"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_fail "发现 $EMPTY_WORDS 个空单词"
fi

# 检查重复单词
DUPLICATE_WORDS=$(sqlite3 "$DB_FILE" "
SELECT COUNT(*) FROM (
    SELECT book_id, word_id, COUNT(*) as cnt
    FROM words
    GROUP BY book_id, word_id
    HAVING cnt > 1
);")
TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ "$DUPLICATE_WORDS" -eq 0 ]; then
    print_pass "无重复单词"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_fail "发现 $DUPLICATE_WORDS 个重复单词"
fi

# 检查孤立单词（词库不存在）
ORPHAN_WORDS=$(sqlite3 "$DB_FILE" "
SELECT COUNT(*) FROM words w
WHERE NOT EXISTS (SELECT 1 FROM books b WHERE b.id = w.book_id);
")
TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ "$ORPHAN_WORDS" -eq 0 ]; then
    print_pass "无孤立单词"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_fail "发现 $ORPHAN_WORDS 个孤立单词"
fi

echo ""

# 5. 索引验证
echo "5. 索引验证"
echo "----------"

INDEXES=(
    "idx_books_category"
    "idx_books_is_active"
    "idx_words_word"
    "idx_words_book_id"
    "idx_words_book_word"
)

for index in "${INDEXES[@]}"; do
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if sqlite3 "$DB_FILE" "SELECT name FROM sqlite_master WHERE type='index' AND name='$index';" | grep -q "$index"; then
        print_pass "索引 $index 存在"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_warning "索引 $index 不存在"
    fi
done

echo ""

# 6. 性能测试
echo "6. 性能测试"
echo "----------"

# 查询测试
START_TIME=$(date +%s%N)
sqlite3 "$DB_FILE" "SELECT * FROM words WHERE word LIKE '%test%' LIMIT 10;" > /dev/null
END_TIME=$(date +%s%N)
QUERY_TIME=$(( (END_TIME - START_TIME) / 1000000 ))  # 转换为毫秒

TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ "$QUERY_TIME" -lt 100 ]; then
    print_pass "查询性能: ${QUERY_TIME}ms (< 100ms)"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_warning "查询性能: ${QUERY_TIME}ms (>= 100ms)"
fi

# 数据库大小
DB_SIZE=$(du -h "$DB_FILE" | cut -f1)
echo ""
echo "数据库大小: $DB_SIZE"

echo ""
echo "验证结果"
echo "========"
echo "通过: $PASSED_TESTS / $TOTAL_TESTS"

if [ "$PASSED_TESTS" -eq "$TOTAL_TESTS" ]; then
    echo -e "${GREEN}所有测试通过！${NC}"
    exit 0
else
    FAILED=$((TOTAL_TESTS - PASSED_TESTS))
    echo -e "${RED}失败: $FAILED 个测试${NC}"
    exit 1
fi