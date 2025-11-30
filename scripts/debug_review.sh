#!/bin/bash

# ============================================
# 复习功能调试脚本
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
echo "复习功能调试信息"
echo "==================================="
echo "数据库: $DB_FILE"
echo "词库ID: $BOOK_ID"
echo ""

# 1. 检查今天的日期
echo "1. 当前日期:"
sqlite3 "$DB_FILE" "SELECT DATE('now') as today;"
echo ""

# 2. 检查是否有学习记录
echo "2. 今日学习记录:"
sqlite3 "$DB_FILE" "
SELECT 
    COUNT(*) as count,
    study_type
FROM study_records
WHERE book_id = '$BOOK_ID'
  AND DATE(studied_at) = DATE('now')
GROUP BY study_type;
"
echo ""

# 3. 检查复习计划
echo "3. 复习计划统计:"
sqlite3 "$DB_FILE" "
SELECT 
    COUNT(*) as total,
    mastery_level,
    CASE 
        WHEN mastery_level = 0 THEN '未学习'
        WHEN mastery_level = 1 THEN '学习中'
        WHEN mastery_level = 2 THEN '已掌握'
    END as status
FROM review_schedule
WHERE book_id = '$BOOK_ID'
GROUP BY mastery_level;
"
echo ""

# 4. 检查待复习单词
echo "4. 今日待复习单词 (前10个):"
sqlite3 "$DB_FILE" "
SELECT 
    rs.word_id,
    w.word,
    rs.next_review_date,
    rs.review_interval,
    rs.repetition_count,
    rs.mastery_level,
    CASE 
        WHEN rs.next_review_date <= DATE('now') THEN '是'
        ELSE '否'
    END as should_review_today
FROM review_schedule rs
JOIN words w ON rs.word_id = w.id
WHERE rs.book_id = '$BOOK_ID'
  AND rs.mastery_level < 2
ORDER BY rs.next_review_date ASC
LIMIT 10;
"
echo ""

# 5. 统计应该复习的单词数
echo "5. 应该复习的单词数:"
sqlite3 "$DB_FILE" "
SELECT COUNT(*) as count
FROM review_schedule
WHERE book_id = '$BOOK_ID'
  AND next_review_date <= DATE('now')
  AND mastery_level < 2;
"
echo ""

# 6. 检查复习日期分布
echo "6. 复习日期分布 (未来7天):"
sqlite3 "$DB_FILE" "
SELECT 
    next_review_date,
    COUNT(*) as count
FROM review_schedule
WHERE book_id = '$BOOK_ID'
  AND mastery_level < 2
  AND next_review_date BETWEEN DATE('now') AND DATE('now', '+7 days')
GROUP BY next_review_date
ORDER BY next_review_date;
"
echo ""

# 7. 检查最近的复习记录
echo "7. 最近的复习记录 (前5条):"
sqlite3 "$DB_FILE" "
SELECT 
    sr.word_id,
    w.word,
    sr.study_type,
    sr.result,
    DATE(sr.studied_at) as studied_date
FROM study_records sr
JOIN words w ON sr.word_id = w.id
WHERE sr.book_id = '$BOOK_ID'
ORDER BY sr.studied_at DESC
LIMIT 5;
"
echo ""

# 8. 问题诊断
echo "==================================="
echo "问题诊断"
echo "==================================="

LEARNED_COUNT=$(sqlite3 "$DB_FILE" "
SELECT COUNT(*) FROM review_schedule 
WHERE book_id = '$BOOK_ID';
")

TODAY_REVIEW_COUNT=$(sqlite3 "$DB_FILE" "
SELECT COUNT(*) FROM review_schedule 
WHERE book_id = '$BOOK_ID'
  AND next_review_date <= DATE('now')
  AND mastery_level < 2;
")

echo "- 已学习单词数: $LEARNED_COUNT"
echo "- 今日应复习: $TODAY_REVIEW_COUNT"

if [ "$LEARNED_COUNT" -eq 0 ]; then
    echo ""
    echo "❌ 问题: 还没有学习任何单词"
    echo "   解决: 先去学习页面学习一些单词"
elif [ "$TODAY_REVIEW_COUNT" -eq 0 ]; then
    echo ""
    echo "❌ 问题: 没有需要复习的单词"
    echo "   可能原因:"
    echo "   1. 今天刚学的单词，复习日期是明天"
    echo "   2. 所有单词都已掌握 (mastery_level = 2)"
    
    # 检查明天需要复习的
    TOMORROW_COUNT=$(sqlite3 "$DB_FILE" "
    SELECT COUNT(*) FROM review_schedule 
    WHERE book_id = '$BOOK_ID'
      AND next_review_date = DATE('now', '+1 day')
      AND mastery_level < 2;
    ")
    
    echo ""
    echo "   明天需要复习: $TOMORROW_COUNT 个单词"
    
    if [ "$TOMORROW_COUNT" -gt 0 ]; then
        echo ""
        echo "🔧 临时解决方案: 手动修改复习日期为今天"
        echo "   运行以下命令:"
        echo "   sqlite3 $DB_FILE \"UPDATE review_schedule SET next_review_date = DATE('now') WHERE book_id = '$BOOK_ID' AND mastery_level < 2;\""
    fi
else
    echo ""
    echo "✅ 正常: 有 $TODAY_REVIEW_COUNT 个单词可以复习"
fi

echo ""
echo "==================================="