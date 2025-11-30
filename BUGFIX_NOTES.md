# Bug 修复说明

## 修复日期
2025-11-30

## 问题描述

### Bug 1: ReviewQuality 枚举值不一致

**问题：**
- 代码中 `ReviewQuality` 枚举定义为：Again=0, Hard=2, Good=3, Easy=5
- 但 SM-2 算法标准应该是：Again=0, Hard=3, Good=4, Easy=5

**影响：**
- SM-2 算法计算不准确
- 测试用例无法通过
- Hard 质量会触发重置逻辑（因为 2 < 3）

**修复：**
```cpp
// 修复前
enum class ReviewQuality {
    Again = 0,
    Hard = 2,    // ❌ 错误
    Good = 3,
    Easy = 5
};

// 修复后
enum class ReviewQuality {
    Again = 0,
    Hard = 3,    // ✅ 正确
    Good = 4,    // ✅ 正确
    Easy = 5
};
```

**修改文件：**
- `src/domain/entities.h`
- `src/application/services/sm2_scheduler.h`
- `src/application/services/sm2_scheduler.cpp`
- `tests/unit/test_sm2_algorithm.cpp`

---

### Bug 2: 复习日期计算逻辑错误

**问题：**
- 学习新单词后，初始化复习计划时 `nextReviewDate` 设置为今天
- 导致今天学习的单词，今天就需要复习（不合理）
- 正确逻辑应该是：今天学习，明天开始复习

**影响：**
- 用户体验不佳（刚学完就要复习）
- 集成测试失败

**修复：**
```cpp
// 修复前
void SM2Scheduler::initializeSchedule(int wordId, const QString& bookId) {
    Domain::ReviewPlan plan;
    // ...
    plan.nextReviewDate = QDate::currentDate();  // ❌ 今天
    // ...
}

// 修复后
void SM2Scheduler::initializeSchedule(int wordId, const QString& bookId) {
    Domain::ReviewPlan plan;
    // ...
    plan.nextReviewDate = QDate::currentDate().addDays(1);  // ✅ 明天
    // ...
}
```

**修改文件：**
- `src/application/services/sm2_scheduler.cpp`
- `tests/integration/test_study_flow.cpp`

---

## SM-2 算法行为变化

### 修复前

| 质量 | 值 | 行为 |
|------|-----|------|
| Again | 0 | 重置间隔 ✅ |
| Hard | 2 | **重置间隔** ❌ (因为 2 < 3) |
| Good | 3 | 增加间隔 ✅ |
| Easy | 5 | 增加间隔 ✅ |

### 修复后

| 质量 | 值 | 行为 |
|------|-----|------|
| Again | 0 | 重置间隔 ✅ |
| Hard | 3 | **增加间隔**（但EF降低） ✅ |
| Good | 4 | 增加间隔 ✅ |
| Easy | 5 | 增加间隔 ✅ |

**关键差异：**
- `Hard(3)` 现在不会重置学习进度，只是 EF 会降低，间隔增长变慢
- 这更符合 SuperMemo SM-2 标准：只有 `Again(0-2)` 才重置

---

## 测试用例修改

### test_sm2_algorithm.cpp

**修改测试：**
```cpp
// 修复前
TEST_F(SM2AlgorithmTest, QualityHard_DecreasesEF) {
    auto result = calculate(6, 2.5, 2, ReviewQuality::Hard);
    EXPECT_LT(result.easinessFactor, 2.5);
}

// 修复后
TEST_F(SM2AlgorithmTest, QualityHard_DecreasesEF_ButContinues) {
    auto result = calculate(6, 2.5, 2, ReviewQuality::Hard);
    
    EXPECT_LT(result.easinessFactor, 2.5);  // EF 降低
    EXPECT_EQ(result.repetitionCount, 3);    // 但继续复习
    EXPECT_GT(result.interval, 0);           // 间隔不重置
}
```

### test_study_flow.cpp

**修改测试：**
```cpp
// 修复前
TEST_F(StudyFlowIntegrationTest, ReviewSession) {
    // 学习单词...
    
    // 开始复习会话
    auto reviewSession = service->startSession(...);
    
    EXPECT_EQ(reviewSession.wordIds.size(), 3);  // ❌ 错误假设
}

// 修复后
TEST_F(StudyFlowIntegrationTest, ReviewSession) {
    // 1. 学习单词...
    
    // 2. 验证今天不需要复习
    auto reviewSessionToday = service->startSession(...);
    EXPECT_EQ(reviewSessionToday.wordIds.size(), 0);  // ✅ 今天刚学的
    
    // 3. 手动修改日期为今天（模拟时间流逝）
    for (int wordId : learnSession.wordIds) {
        ReviewPlan plan = scheduleRepo->get(wordId);
        plan.nextReviewDate = QDate::currentDate();
        scheduleRepo->save(plan);
    }
    
    // 4. 现在应该有待复习的了
    auto reviewSession = service->startSession(...);
    EXPECT_EQ(reviewSession.wordIds.size(), 3);  // ✅ 正确
}
```

---

## 验证修复

### 运行测试

```bash
# 重新构建
make clean
make build

# 运行 SM-2 算法测试
cd build
./tests/test_sm2_algorithm

# 运行学习流程测试
./tests/test_study_flow

# 运行所有测试
ctest --output-on-failure
```

### 预期结果

```
[==========] Running 12 tests from SM2AlgorithmTest
...
[  PASSED  ] 12 tests.

[==========] Running 11 tests from StudyFlowIntegrationTest
...
[  PASSED  ] 11 tests.
```

---

## 实际行为示例

### 场景 1: 学习新单词

```
时间: 2025-11-30 (周六)

用户学习 "apple"
结果: 认识 (known=true)

复习计划:
- next_review_date: 2025-12-01 (周日)  ✅ 明天
- interval: 1
- repetition_count: 1
```

### 场景 2: 复习单词 - Hard

```
时间: 2025-12-01 (周日)

用户复习 "apple"
质量: Hard (觉得有点难)

更新后:
- next_review_date: 2025-12-02 (周一)
- interval: 依然增长 (约 1.3 天)
- EF: 降低到 2.3 (下次增长变慢)
- repetition_count: 2  ✅ 继续计数
```

### 场景 3: 复习单词 - Again

```
时间: 2025-12-02 (周一)

用户复习 "apple"
质量: Again (完全忘了)

更新后:
- next_review_date: 2025-12-03 (周二)
- interval: 1  ✅ 重置
- EF: 降低到 2.0
- repetition_count: 0  ✅ 重置
```

---

## 回归测试检查清单

- [x] SM-2 算法所有测试通过
- [x] 学习流程集成测试通过
- [x] 词库导入测试通过
- [x] Repository 单元测试通过
- [x] 代码逻辑与注释一致
- [x] 枚举值符合 SM-2 标准

---

## 相关文件

### 修改的文件
- `src/domain/entities.h`
- `src/application/services/sm2_scheduler.h`
- `src/application/services/sm2_scheduler.cpp`
- `tests/unit/test_sm2_algorithm.cpp`
- `tests/integration/test_study_flow.cpp`

### 影响的功能
- 学习新单词
- 复习单词
- 复习计划更新
- SM-2 算法计算

---

## 注意事项

**如果已有旧数据：**
- 旧的复习计划可能使用了错误的质量值
- 建议重新导入词库或清空学习记录
- 或运行数据迁移脚本（如需要）

**升级指南：**
```bash
# 选项 1: 清空学习数据
sqlite3 wordmaster.db "DELETE FROM study_records;"
sqlite3 wordmaster.db "DELETE FROM review_schedule;"

# 选项 2: 重新创建数据库
rm wordmaster.db
./wordmaster_cli --import <词库JSON>
```

---

## 总结

这次修复确保了：
1. ✅ SM-2 算法符合标准
2. ✅ 复习日期逻辑正确
3. ✅ 所有测试通过
4. ✅ 用户体验合理

**核心改进：**
- Hard 质量不再重置学习进度（更合理）
- 今天学习的单词明天才复习（符合认知规律）
- 测试用例更准确反映实际行为