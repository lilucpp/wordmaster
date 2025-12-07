#!/bin/bash

# ============================================
# 修复复习日期脚本
# 将明天的复习日期改为今天（用于测试）
# ============================================

if [ $# -lt 1 ]; then
    echo "用法: $0 <数据库文件> [词库ID]"
    echo "示例: $0 build/wordmaster.db cet4"
    exit 1
fi

DB_FILE="$1"
BOOK_ID="${2:-cet4}"

if [ ! -f "$DB_FILE" ]; then
    echo "错误: 数据库文件不存在: $DB_FILE"
    exit 1
fi

echo "==================================="
echo "修复复习日期"
echo "==================================="
echo "数据库: $DB_FILE"
echo "词库ID: $BOOK_ID"
echo ""

# 检查需要修改的单词数
COUNT=$(sqlite3 "$DB_FILE" "
SELECT COUNT(*) FROM review_schedule 
WHERE book_id = '$BOOK_ID'
  AND next_review_date > DATE('now')
  AND mastery_level < 2;
")

echo "需要修改的单词数: $COUNT"

if [ "$COUNT" -eq 0 ]; then
    echo "没有需要修改的单词"
    exit 0
fi

echo ""
read -p "确认将这些单词的复习日期改为今天？(y/N): " -n 1 -r
echo

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "已取消"
    exit 0
fi

# 执行修改
sqlite3 "$DB_FILE" "
UPDATE review_schedule 
SET next_review_date = DATE('now')
WHERE book_id = '$BOOK_ID'
  AND next_review_date > DATE('now')
  AND mastery_level < 2;
"

echo ""
echo "✅ 已修改 $COUNT 个单词的复习日期"
echo ""

# 验证
REVIEW_COUNT=$(sqlite3 "$DB_FILE" "
SELECT COUNT(*) FROM review_schedule 
WHERE book_id = '$BOOK_ID'
  AND next_review_date <= DATE('now')
  AND mastery_level < 2;
")

echo "现在可复习的单词数: $REVIEW_COUNT"
echo ""
echo "现在可以在应用中开始复习了！"