#!/usr/bin/env python3
"""
批量将 Python 2 脚本转换为 Python 3 脚本
但其实智能解决很小一部分问题，只能处理语法问题，外部库依赖处理不来
"""

import os
import subprocess
import shutil
from pathlib import Path

def convert_python2_to_python3(file_list, backup=True):
    """
    将 Python 2 文件转换为 Python 3
    
    Args:
        file_list: 要转换的文件列表
        backup: 是否创建备份文件
    """
    converted_files = []
    failed_files = []
    
    for file_path in file_list:
        if not os.path.exists(file_path):
            print(f"警告: 文件 {file_path} 不存在，跳过")
            continue
            
        print(f"正在转换: {file_path}")
        
        try:
            # 创建备份
            if backup:
                backup_path = file_path + '.py2backup'
                shutil.copy2(file_path, backup_path)
                print(f"  已创建备份: {backup_path}")
            
            # 使用 2to3 进行转换
            # -w 参数表示直接写入原文件
            # -n 表示不备份（因为我们已经手动备份了）
            result = subprocess.run([
                '2to3', 
                '-w', 
                '-n',  # 不创建备份（因为我们已经手动备份了）
                '--no-diffs',
                file_path
            ], capture_output=True, text=True, timeout=60)
            
            if result.returncode == 0:
                print(f"  ✓ 转换成功: {file_path}")
                converted_files.append(file_path)
                
                # 显示转换统计
                if "RefactoringTool: Refactored" in result.stdout:
                    lines = result.stdout.split('\n')
                    for line in lines:
                        if "RefactoringTool: Refactored" in line:
                            print(f"  {line.strip()}")
            else:
                print(f"  ✗ 转换失败: {file_path}")
                print(f"  错误信息: {result.stderr}")
                failed_files.append(file_path)
                
        except subprocess.TimeoutExpired:
            print(f"  ✗ 转换超时: {file_path}")
            failed_files.append(file_path)
        except Exception as e:
            print(f"  ✗ 转换异常: {file_path}")
            print(f"  异常信息: {e}")
            failed_files.append(file_path)
    
    return converted_files, failed_files

def main():
    # 要转换的文件列表
    files_to_convert = [
        "analysis.py",
        "bee_simulator.py", 
        "central_complex.py",
        "cx_basic.py",
        "cx_rate.py",
        "plotter.py",
        "trials.py"
    ]
    
    print("开始批量转换 Python 2 到 Python 3")
    print("=" * 50)
    
    # 检查 2to3 工具是否可用
    try:
        subprocess.run(['2to3', '--help'], capture_output=True, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("错误: 2to3 工具未找到，请确保已安装 Python 3 并配置好环境变量")
        return
    
    # 检查文件是否存在
    existing_files = [f for f in files_to_convert if os.path.exists(f)]
    missing_files = [f for f in files_to_convert if not os.path.exists(f)]
    
    if missing_files:
        print("以下文件不存在，将跳过:")
        for f in missing_files:
            print(f"  - {f}")
        print()
    
    if not existing_files:
        print("没有找到要转换的文件")
        return
    
    # 确认转换
    print("将要转换的文件:")
    for f in existing_files:
        print(f"  - {f}")
    
    response = input("\n是否继续转换？(y/n): ").lower().strip()
    if response not in ['y', 'yes']:
        print("转换已取消")
        return
    
    # 执行转换
    converted, failed = convert_python2_to_python3(existing_files)
    
    # 输出结果摘要
    print("\n" + "=" * 50)
    print("转换完成!")
    print(f"成功转换: {len(converted)} 个文件")
    print(f"转换失败: {len(failed)} 个文件")
    
    if converted:
        print("\n成功转换的文件:")
        for f in converted:
            print(f"  ✓ {f}")
    
    if failed:
        print("\n转换失败的文件:")
        for f in failed:
            print(f"  ✗ {f}")

if __name__ == "__main__":
    main()