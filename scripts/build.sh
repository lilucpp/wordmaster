#!/bin/bash

# ============================================
# WordMaster 构建脚本
# ============================================

set -e  # 遇到错误立即退出

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 项目根目录
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# 解析命令行参数
BUILD_TYPE="Debug"
BUILD_TESTS="ON"
ENABLE_COVERAGE="OFF"
CLEAN_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        --coverage)
            ENABLE_COVERAGE="ON"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --release       Build in Release mode (default: Debug)"
            echo "  --no-tests      Do not build tests"
            echo "  --coverage      Enable code coverage"
            echo "  --clean         Clean build directory before building"
            echo "  --help          Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_info "WordMaster Build Script"
print_info "========================"
print_info "Build Type: ${BUILD_TYPE}"
print_info "Build Tests: ${BUILD_TESTS}"
print_info "Coverage: ${ENABLE_COVERAGE}"

# 清理构建目录
if [ "$CLEAN_BUILD" = true ]; then
    print_info "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
fi

# 创建构建目录
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# 检查依赖
print_info "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    print_error "CMake not found. Please install CMake."
    exit 1
fi

if ! command -v qmake &> /dev/null; then
    print_error "Qt5 not found. Please install Qt5."
    exit 1
fi

if [ "$BUILD_TESTS" = "ON" ]; then
    if ! pkg-config --exists gtest; then
        print_warning "Google Test not found. Tests may not build."
    fi
fi

# 配置
print_info "Configuring project..."
cmake .. \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_TESTS="${BUILD_TESTS}" \
    -DENABLE_COVERAGE="${ENABLE_COVERAGE}"

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed"
    exit 1
fi

# 编译
print_info "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    print_error "Build failed"
    exit 1
fi

print_info "Build completed successfully!"

# 运行测试
if [ "$BUILD_TESTS" = "ON" ]; then
    print_info "Running tests..."
    ctest --output-on-failure
    
    if [ $? -ne 0 ]; then
        print_error "Tests failed"
        exit 1
    fi
    
    print_info "All tests passed!"
fi

# 生成覆盖率报告
if [ "$ENABLE_COVERAGE" = "ON" ]; then
    print_info "Generating coverage report..."
    
    if command -v lcov &> /dev/null; then
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --remove coverage.info '*/tests/*' --output-file coverage.info
        
        if command -v genhtml &> /dev/null; then
            genhtml coverage.info --output-directory coverage_html
            print_info "Coverage report generated: ${BUILD_DIR}/coverage_html/index.html"
        fi
    else
        print_warning "lcov not found. Coverage report not generated."
    fi
fi

print_info ""
print_info "Build artifacts:"
print_info "  Executable: ${BUILD_DIR}/WordMaster"
print_info "  Tests: ${BUILD_DIR}/tests/"
print_info ""
print_info "To run the application:"
print_info "  ${BUILD_DIR}/WordMaster"