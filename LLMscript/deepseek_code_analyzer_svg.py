#!/usr/bin/env python3
"""
Multi-SVG Diagram Generator for Code Files (DeepSeek Version)
Modified to process only .c/.cpp files, exclude *mod.c and .cmd files
"""

import os
import requests
import time
import argparse
import random
import logging
import json
import re
from datetime import datetime
from dotenv import load_dotenv

# Set up logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("deepseek_svg_generation.log"),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

# Load API Key from .env file
load_dotenv()
api_key = os.getenv("DEEPSEEK_API_KEY")

if not api_key:
    raise ValueError("Missing DeepSeek API key. Add it to a .env file as DEEPSEEK_API_KEY=your_key_here")

# DeepSeek API configuration
DEEPSEEK_API_URL = "https://api.deepseek.com/v1/chat/completions"
SUPPORTED_EXTENSIONS = ['.cpp', '.c']  # Modified for C/C++ only
STATE_FILE = "deepseek_svg_generation_state.json"

class DeepSeekClient:
    """Client for interacting with DeepSeek API"""
    
    def __init__(self, api_key):
        self.api_key = api_key
        self.headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {api_key}"
        }
    
    def generate_completion(self, model, messages, max_tokens=4096, temperature=0.2):
        """Send a request to DeepSeek API and return the response"""
        payload = {
            "model": model,
            "messages": messages,
            "max_tokens": max_tokens,
            "temperature": temperature
        }
        
        response = requests.post(
            DEEPSEEK_API_URL,
            headers=self.headers,
            json=payload
        )
        
        if response.status_code != 200:
            raise Exception(f"API Error: {response.status_code} {response.text}")
            
        return response.json()

client = DeepSeekClient(api_key)

def extract_code_essence(file_content, max_length=3000):
    """Extract the essential parts of the code to reduce prompt size."""
    if len(file_content) <= max_length:
        return file_content
    
    lines = file_content.split('\n')
    top_section = '\n'.join(lines[:int(len(lines) * 0.2)])
    
    function_pattern = r'(def\s+\w+|class\s+\w+|\w+\s*\(\)|\w+::\w+)'
    functions = re.findall(function_pattern, file_content)
    
    main_section = ""
    main_patterns = ['int main', 'void main', 'public static void main']
    for pattern in main_patterns:
        if pattern in file_content:
            main_match = re.search(pattern + '.*?\{', file_content, re.DOTALL)
            if main_match:
                start_pos = main_match.start()
                main_section = file_content[start_pos:min(start_pos + 800, len(file_content))]
    
    result = top_section
    if len(functions) > 0:
        result += "\n\n// Key function definitions:\n"
        result += "\n".join([f"// - {func}" for func in functions[:15]])
    
    if main_section:
        result += "\n\n// Main function:\n" + main_section
    
    if len(result) > max_length:
        result = result[:max_length] + "\n\n// [Code truncated due to length...]"
    
    return result

def create_prompt(file_content, filename, max_code_length=3000):
    """Generate a DeepSeek-friendly prompt for C/C++ code"""
    language = "C++" if filename.endswith('.cpp') else "C"
    code_essence = extract_code_essence(file_content, max_code_length)
    
    prompt = f"""
    Create MULTIPLE SVG diagrams (6-8) explaining this {language} code.
    Focus areas:
    1. Overall architecture
    2. Core algorithm flow
    3. Data structures
    4. Memory management
    5. Function interactions
    6. Error handling
    
    Requirements:
    - ONLY valid SVG code output
    - 1200x800 dimensions
    - Clear diagram titles
    - Separate SVGs with "---"
    
    File: {filename}
    
    Code:
    ```
    {code_essence}
    ```
    """
    return prompt.strip()

def exponential_backoff(attempt, base_delay=2, max_delay=30):
    delay = min(base_delay * (2 ** attempt), max_delay)
    return delay * random.uniform(0.8, 1.2)

def request_svg_diagrams(prompt, model_name, max_retries=5):
    for attempt in range(max_retries):
        try:
            messages = [{"role": "user", "content": prompt}]
            response = client.generate_completion(
                model=model_name,
                max_tokens=4096,
                temperature=0.2,
                messages=messages
            )
            
            if "choices" in response and len(response["choices"]) > 0:
                return response["choices"][0]["message"]["content"].strip()
            
        except requests.exceptions.ConnectionError:
            delay = exponential_backoff(attempt)
            logger.warning(f"Connection error. Retrying in {delay:.1f}s...")
            time.sleep(delay)
        
        except Exception as e:
            logger.error(f"Error: {str(e)}")
            if attempt < max_retries - 1:
                delay = exponential_backoff(attempt)
                time.sleep(delay)
    
    return None

def extract_multiple_svgs(response_text):
    svg_matches = re.findall(r'<svg[\s\S]*?<\/svg>', response_text)
    return svg_matches if svg_matches else [response_text]

def ensure_imgs_directory(file_dir):
    imgs_dir = os.path.join(file_dir, "imgs")
    os.makedirs(imgs_dir, exist_ok=True)
    return imgs_dir

def save_multiple_svgs(svg_contents, base_filepath):
    file_dir = os.path.dirname(base_filepath)
    base_filename = os.path.basename(base_filepath)
    name_without_ext = os.path.splitext(base_filename)[0]
    
    imgs_dir = ensure_imgs_directory(file_dir)
    saved_files = []
    
    for i, svg_content in enumerate(svg_contents):
        svg_filename = f"{name_without_ext}_diagram{i+1}.svg"
        svg_filepath = os.path.join(imgs_dir, svg_filename)
        
        try:
            with open(svg_filepath, 'w', encoding='utf-8') as f:
                f.write(svg_content)
            saved_files.append(svg_filepath)
            logger.info(f"Saved: {svg_filepath}")
        except Exception as e:
            logger.error(f"Save error: {e}")
    
    return saved_files

def load_state():
    if os.path.exists(STATE_FILE):
        try:
            with open(STATE_FILE, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"State load error: {e}")
    
    return {"pending": [], "completed": [], "failed": [], "last_run": None}

def save_state(state):
    state["last_run"] = datetime.now().isoformat()
    try:
        with open(STATE_FILE, 'w') as f:
            json.dump(state, f, indent=2)
    except Exception as e:
        logger.error(f"State save error: {e}")

def find_eligible_files(root_dir, skip_existing=True):
    """Find files with enhanced exclusion rules"""
    state = load_state()
    state["pending"] = []
    
    for subdir, _, files in os.walk(root_dir):
        for file in files:
            ext = os.path.splitext(file)[1].lower()
            file_lower = file.lower()
            
            # Exclusion criteria
            if (ext not in SUPPORTED_EXTENSIONS or 
                file_lower.endswith(".cmd") or 
                file_lower.endswith("mod.c")):
                continue
            
            file_path = os.path.join(subdir, file)
            name_without_ext = os.path.splitext(file)[0]
            imgs_dir = os.path.join(subdir, "imgs")
            
            if skip_existing and os.path.exists(imgs_dir):
                existing = [f for f in os.listdir(imgs_dir) 
                           if f.startswith(name_without_ext) and f.endswith('.svg')]
                if existing:
                    if file_path not in state["completed"]:
                        state["completed"].append(file_path)
                    continue
                
            if file_path not in state["completed"] + state["failed"]:
                state["pending"].append(file_path)
    
    save_state(state)
    logger.info(f"Files: {len(state['pending'])} pending, {len(state['completed'])} done")
    return state

def process_file(file_path, model_name, max_code_length):
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        prompt = create_prompt(content, os.path.basename(file_path), max_code_length)
        response = request_svg_diagrams(prompt, model_name)
        
        if not response:
            return "failed"
        
        svgs = extract_multiple_svgs(response)
        if not svgs:
            return "failed"
        
        saved = save_multiple_svgs(svgs, file_path)
        return "completed" if saved else "failed"
    
    except Exception as e:
        logger.error(f"Processing failed: {str(e)}")
        return "failed"

def process_batch(batch_size, model_name, throttle, max_code_length):
    state = load_state()
    processed = 0
    
    for _ in range(min(batch_size, len(state["pending"]))):
        if not state["pending"]:
            break
        
        file_path = state["pending"].pop(0)
        result = process_file(file_path, model_name, max_code_length)
        
        if result == "completed":
            state["completed"].append(file_path)
        else:
            state["failed"].append(file_path)
        
        save_state(state)
        processed += 1
        
        if throttle > 0 and state["pending"]:
            time.sleep(throttle)
    
    return processed

def main_process(root_dir, model_name, batch_size=5, throttle=5, skip_existing=True, max_code_length=3000):
    find_eligible_files(root_dir, skip_existing)
    total = 0
    
    while True:
        state = load_state()
        if not state["pending"]:
            break
        
        batch_processed = process_batch(batch_size, model_name, throttle, max_code_length)
        total += batch_processed
        if batch_processed == 0:
            break
    
    logger.info(f"Total processed: {total}")

def main():
    parser = argparse.ArgumentParser(description='C/C++ SVG Generator via DeepSeek')
    parser.add_argument('--dir', required=True, help='Target directory')
    parser.add_argument('--model', default='deepseek-coder', help='Model name')
    parser.add_argument('--batch', type=int, default=5, help='Files per batch')
    parser.add_argument('--throttle', type=int, default=5, help='Delay between files')
    parser.add_argument('--process-all', action='store_true', help='Overwrite existing')
    parser.add_argument('--max-length', type=int, default=3000, help='Code input limit')
    parser.add_argument('--reset', action='store_true', help='Reset processing state')
    
    args = parser.parse_args()
    
    if args.reset:
        if os.path.exists(STATE_FILE):
            os.remove(STATE_FILE)
    
    try:
        main_process(
            args.dir,
            args.model,
            batch_size=args.batch,
            throttle=args.throttle,
            skip_existing=not args.process_all,
            max_code_length=args.max_length
        )
    except KeyboardInterrupt:
        logger.info("Interrupted by user")
    except Exception as e:
        logger.critical(f"Fatal error: {str(e)}")

if __name__ == "__main__":
    main()