# WordMaster Makefile
# 提供便捷的开发命令

.PHONY: help build test clean run cli import list stats verify

# 默认目标
help:
	@echo "WordMaster 开发命令"
	@echo "===================="
	@echo ""
	@echo "构建命令:"
	@echo "  make build          - 构建项目（Debug）"
	@echo "  make build-release  - 构建项目（Release）"
	@echo "  make clean          - 清理构建文件"
	@echo ""
	@echo "测试命令:"
	@echo "  make test           - 运行所有测试"
	@echo "  make test-unit      - 运行单元测试"
	@echo "  make test-integration - 运行集成测试"
	@echo ""
	@echo "CLI 命令:"
	@echo "  make cli            - 进入 CLI 交互模式"
	@echo "  make import DATA=<path> - 导入词库数据"
	@echo "  make list           - 列出所有词库"
	@echo "  make stats BOOK=<id> - 查看词库统计"
	@echo "  make verify         - 验证数据完整性"
	@echo ""
	@echo "开发命令:"
	@echo "  make run            - 运行主程序"
	@echo "  make coverage       - 生成代码覆盖率报告"
	@echo ""

# 构建
build:
	@./scripts/build.sh

build-release:
	@./scripts/build.sh --release

clean:
	@rm -rf build
	@echo "清理完成"

# 测试
test: build
	@cd build && ctest --output-on-failure

test-unit: build
	@cd build && ctest --output-on-failure -R "test_.*"

test-integration: build
	@cd build && ctest --output-on-failure -R "integration_.*"

# CLI 工具
cli: build
	@cd build && ./wordmaster_cli --help

import: build
ifndef DATA
	@echo "错误: 请指定数据文件路径"
	@echo "用法: make import DATA=/path/to/word_meta.json"
	@exit 1
endif
	@./scripts/quick_test.sh $(DATA)

list: build
	@cd build && ./wordmaster_cli --list

stats: build
ifndef BOOK
	@echo "错误: 请指定词库ID"
	@echo "用法: make stats BOOK=cet4"
	@exit 1
endif
	@cd build && ./wordmaster_cli --stats $(BOOK)

verify: build
	@./scripts/verify_data.sh build/wordmaster.db

# 运行
run: build
	@cd build && ./WordMaster

# 代码覆盖率
coverage:
	@./scripts/build.sh --coverage
	@echo "覆盖率报告: build/coverage_html/index.html"

# 帮助文档
docs:
	@echo "文档列表:"
	@echo "  README.md           - 项目说明"
	@echo "  README_DEV.md       - 开发文档"
	@echo "  CLI_USAGE_GUIDE.md  - CLI 使用指南"
	@echo "  BUILD_AND_TEST.md   - 构建和测试指南"

# 快速开始
quickstart:
	@echo "快速开始 WordMaster"
	@echo "==================="
	@echo ""
	@echo "1. 构建项目:"
	@echo "   make build"
	@echo ""
	@echo "2. 运行测试:"
	@echo "   make test"
	@echo ""
	@echo "3. 导入真实数据:"
	@echo "   make import DATA=/path/to/1764429285470_recommend_word.json"
	@echo ""
	@echo "4. 查看词库:"
	@echo "   make list"
	@echo ""
	@echo "5. 验证数据:"
	@echo "   make verify"