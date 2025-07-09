# 使用Ubuntu 22.04作为基础镜像
FROM ubuntu:22.04

# 设置环境变量避免交互式安装
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# 安装基础依赖项（与原虚拟机配置保持一致）
RUN apt-get update && apt-get install -y \
    build-essential \
    bison \
    flex \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    nasm \
    qemu-system-x86 \
    wget \
    curl \
    git \
    make \
    gdb \
    vim \
    nano \
    tree \
    xz-utils \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /tools

# 创建交叉编译工具链目录
RUN mkdir -p /tools/opt

# 设置环境变量
ENV PREFIX="/tools/opt/cross"
ENV TARGET=i686-elf
ENV PATH="$PREFIX/bin:$PATH"

# 下载并构建 binutils
WORKDIR /tools/opt
RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.36.1.tar.gz && \
    tar -xzvf binutils-2.36.1.tar.gz && \
    mkdir build-binutils && \
    cd build-binutils && \
    ../binutils-2.36.1/configure \
        --target=$TARGET \
        --prefix="$PREFIX" \
        --with-sysroot \
        --disable-nls \
        --disable-werror && \
    make -j$(nproc) && \
    make install && \
    cd /tools/opt && \
    rm -rf build-binutils binutils-2.36.1.tar.gz

# 下载并构建 gcc
RUN cd /tools/opt && \
    wget https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.gz && \
    tar -xzvf gcc-10.2.0.tar.gz && \
    mkdir build-gcc && \
    cd build-gcc && \
    ../gcc-10.2.0/configure \
        --target=$TARGET \
        --prefix="$PREFIX" \
        --disable-nls \
        --enable-languages=c,c++ \
        --without-headers && \
    make all-gcc -j$(nproc) && \
    make all-target-libgcc -j$(nproc) && \
    make install-gcc && \
    make install-target-libgcc && \
    cd /tools/opt && \
    rm -rf build-gcc gcc-10.2.0.tar.gz

# 验证工具链安装
RUN which i686-elf-gcc && \
    which i686-elf-ld && \
    which i686-elf-as && \
    i686-elf-gcc --version

# 创建项目工作目录
WORKDIR /workspace

# 设置环境变量（与原配置保持一致）
ENV CC=i686-elf-gcc
ENV CXX=i686-elf-g++
ENV AS=i686-elf-as
ENV LD=i686-elf-ld
ENV AR=i686-elf-ar
ENV OBJCOPY=i686-elf-objcopy
ENV OBJDUMP=i686-elf-objdump

# 暴露端口（用于GDB调试等）
EXPOSE 1234 5900

CMD ["/bin/bash"]
