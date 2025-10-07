use serde::{Deserialize, Serialize};
use std::process::Command;
use sysinfo::System;
use which::which;

#[derive(Debug, Serialize, Deserialize)]
pub struct SystemInfo {
    pub platform: String,
    pub arch: String,
    pub gpu: GpuInfo,
    pub cpu: CpuInfo,
    pub ollama: OllamaInfo,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct GpuInfo {
    pub available: bool,
    pub name: Option<String>,
    pub memory: Option<u64>,
    pub driver: Option<String>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct CpuInfo {
    pub cores: usize,
    pub name: String,
    pub memory: u64,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ChatMessage {
    pub role: String,
    pub content: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ChatRequest {
    pub model: String,
    pub messages: Vec<ChatMessage>,
    pub temperature: Option<f64>,
    pub max_tokens: Option<u32>,
    pub stream: Option<bool>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct ChatResponse {
    pub content: String,
    pub usage: Option<TokenUsage>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct TokenUsage {
    pub prompt_tokens: u32,
    pub completion_tokens: u32,
    pub total_tokens: u32,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct OllamaModelInfo {
    pub name: String,
    pub size: Option<u64>,
    pub modified_at: Option<String>,
    pub family: Option<String>,
    pub format: Option<String>,
    pub families: Option<Vec<String>>,
    pub parameter_size: Option<String>,
    pub quantization_level: Option<String>,
}

#[tauri::command]
async fn get_all_ollama_models() -> Result<Vec<OllamaModelInfo>, String> {
    // Read models from models.txt file
    let models_file = std::path::Path::new("models.txt");
    
    let models_content = if models_file.exists() {
        match std::fs::read_to_string(models_file) {
            Ok(content) => content,
            Err(e) => {
                println!("Warning: Could not read models.txt: {}", e);
                return Ok(vec![]);
            }
        }
    } else {
        println!("Warning: models.txt not found, returning empty list");
        return Ok(vec![]);
    };
    
    let mut models = Vec::new();
    
    for line in models_content.lines() {
        let line = line.trim();
        
        // Skip empty lines and comments
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        
        // Parse model name and extract information
        let model_name = line.to_string();
        let (family, parameter_size, quantization_level, estimated_size) = parse_model_info(&model_name);
        
        models.push(OllamaModelInfo {
            name: model_name,
            size: estimated_size,
            modified_at: None,
            family: Some(family.clone()),
            format: Some("gguf".to_string()),
            families: Some(vec![family]),
            parameter_size: Some(parameter_size),
            quantization_level: Some(quantization_level),
        });
    }
    
    Ok(models)
}

fn parse_model_info(model_name: &str) -> (String, String, String, Option<u64>) {
    // Extract family name (everything before the first colon)
    let family = if let Some(colon_pos) = model_name.find(':') {
        model_name[..colon_pos].to_string()
    } else {
        model_name.to_string()
    };
    
    // Extract parameter size and quantization from the model name
    let mut parameter_size = "Unknown".to_string();
    let mut quantization_level = "Default".to_string();
    let mut estimated_size = None;
    
    // Enhanced parameter size detection with more patterns
    if model_name.contains("135m") {
        parameter_size = "135M".to_string();
        estimated_size = Some(200_000_000); // ~200MB
    } else if model_name.contains("22m") {
        parameter_size = "22M".to_string();
        estimated_size = Some(50_000_000); // ~50MB
    } else if model_name.contains("33m") {
        parameter_size = "33M".to_string();
        estimated_size = Some(70_000_000); // ~70MB
    } else if model_name.contains("110m") {
        parameter_size = "110M".to_string();
        estimated_size = Some(200_000_000); // ~200MB
    } else if model_name.contains("360m") {
        parameter_size = "360M".to_string();
        estimated_size = Some(500_000_000); // ~500MB
    } else if model_name.contains("567m") {
        parameter_size = "567M".to_string();
        estimated_size = Some(800_000_000); // ~800MB
    } else if model_name.contains("0.5b") {
        parameter_size = "0.5B".to_string();
        estimated_size = Some(800_000_000); // ~800MB
    } else if model_name.contains("1b") || model_name.contains("1.1b") || model_name.contains("1.3b") || model_name.contains("1.5b") || model_name.contains("1.7b") {
        parameter_size = if model_name.contains("1.1b") { "1.1B".to_string() }
                       else if model_name.contains("1.3b") { "1.3B".to_string() }
                       else if model_name.contains("1.5b") { "1.5B".to_string() }
                       else if model_name.contains("1.7b") { "1.7B".to_string() }
                       else { "1B".to_string() };
        estimated_size = Some(1_200_000_000); // ~1.2GB
    } else if model_name.contains("2b") || model_name.contains("2.4b") || model_name.contains("2.7b") {
        parameter_size = if model_name.contains("2.4b") { "2.4B".to_string() }
                       else if model_name.contains("2.7b") { "2.7b".to_string() }
                       else { "2B".to_string() };
        estimated_size = Some(1_600_000_000); // ~1.6GB
    } else if model_name.contains("3b") || model_name.contains("3.8b") {
        parameter_size = if model_name.contains("3.8b") { "3.8B".to_string() } else { "3B".to_string() };
        estimated_size = Some(2_000_000_000); // ~2GB
    } else if model_name.contains("4b") {
        parameter_size = "4B".to_string();
        estimated_size = Some(2_500_000_000); // ~2.5GB
    } else if model_name.contains("6.7b") {
        parameter_size = "6.7B".to_string();
        estimated_size = Some(3_800_000_000); // ~3.8GB
    } else if model_name.contains("7b") || model_name.contains("7.8b") {
        parameter_size = if model_name.contains("7.8b") { "7.8B".to_string() } else { "7B".to_string() };
        estimated_size = Some(4_100_000_000); // ~4.1GB
    } else if model_name.contains("8b") {
        parameter_size = "8B".to_string();
        estimated_size = Some(4_700_000_000); // ~4.7GB
    } else if model_name.contains("9b") {
        parameter_size = "9B".to_string();
        estimated_size = Some(5_200_000_000); // ~5.2GB
    } else if model_name.contains("10b") || model_name.contains("10.7b") {
        parameter_size = if model_name.contains("10.7b") { "10.7B".to_string() } else { "10B".to_string() };
        estimated_size = Some(6_000_000_000); // ~6GB
    } else if model_name.contains("11b") {
        parameter_size = "11B".to_string();
        estimated_size = Some(6_500_000_000); // ~6.5GB
    } else if model_name.contains("12b") {
        parameter_size = "12B".to_string();
        estimated_size = Some(7_000_000_000); // ~7GB
    } else if model_name.contains("13b") {
        parameter_size = "13B".to_string();
        estimated_size = Some(7_300_000_000); // ~7.3GB
    } else if model_name.contains("14b") {
        parameter_size = "14B".to_string();
        estimated_size = Some(8_000_000_000); // ~8GB
    } else if model_name.contains("15b") {
        parameter_size = "15B".to_string();
        estimated_size = Some(8_500_000_000); // ~8.5GB
    } else if model_name.contains("16b") {
        parameter_size = "16B".to_string();
        estimated_size = Some(9_000_000_000); // ~9GB
    } else if model_name.contains("20b") {
        parameter_size = "20B".to_string();
        estimated_size = Some(11_000_000_000); // ~11GB
    } else if model_name.contains("22b") {
        parameter_size = "22B".to_string();
        estimated_size = Some(12_000_000_000); // ~12GB
    } else if model_name.contains("24b") {
        parameter_size = "24B".to_string();
        estimated_size = Some(13_000_000_000); // ~13GB
    } else if model_name.contains("27b") {
        parameter_size = "27B".to_string();
        estimated_size = Some(15_000_000_000); // ~15GB
    } else if model_name.contains("30b") {
        parameter_size = "30B".to_string();
        estimated_size = Some(17_000_000_000); // ~17GB
    } else if model_name.contains("32b") {
        parameter_size = "32B".to_string();
        estimated_size = Some(18_000_000_000); // ~18GB
    } else if model_name.contains("33b") {
        parameter_size = "33B".to_string();
        estimated_size = Some(19_000_000_000); // ~19GB
    } else if model_name.contains("34b") {
        parameter_size = "34B".to_string();
        estimated_size = Some(20_000_000_000); // ~20GB
    } else if model_name.contains("35b") {
        parameter_size = "35B".to_string();
        estimated_size = Some(20_000_000_000); // ~20GB
    } else if model_name.contains("40b") {
        parameter_size = "40B".to_string();
        estimated_size = Some(23_000_000_000); // ~23GB
    } else if model_name.contains("67b") || model_name.contains("671b") {
        parameter_size = if model_name.contains("671b") { "671B".to_string() } else { "67B".to_string() };
        estimated_size = Some(38_000_000_000); // ~38GB
    } else if model_name.contains("70b") {
        parameter_size = "70B".to_string();
        estimated_size = Some(39_000_000_000); // ~39GB
    } else if model_name.contains("90b") {
        parameter_size = "90B".to_string();
        estimated_size = Some(50_000_000_000); // ~50GB
    } else if model_name.contains("104b") {
        parameter_size = "104B".to_string();
        estimated_size = Some(58_000_000_000); // ~58GB
    } else if model_name.contains("180b") {
        parameter_size = "180B".to_string();
        estimated_size = Some(100_000_000_000); // ~100GB
    } else if model_name.contains("235b") {
        parameter_size = "235B".to_string();
        estimated_size = Some(130_000_000_000); // ~130GB
    } else if model_name.contains("236b") {
        parameter_size = "236B".to_string();
        estimated_size = Some(130_000_000_000); // ~130GB
    } else if model_name.contains("405b") {
        parameter_size = "405B".to_string();
        estimated_size = Some(220_000_000_000); // ~220GB
    } else if model_name.contains("480b") {
        parameter_size = "480B".to_string();
        estimated_size = Some(260_000_000_000); // ~260GB
    }
    
    // Extract quantization level
    if model_name.contains("q4_K_M") {
        quantization_level = "Q4_K_M".to_string();
    } else if model_name.contains("q4_K_S") {
        quantization_level = "Q4_K_S".to_string();
    } else if model_name.contains("q4_0") {
        quantization_level = "Q4_0".to_string();
    } else if model_name.contains("q8_0") {
        quantization_level = "Q8_0".to_string();
    } else if model_name.contains("fp16") {
        quantization_level = "FP16".to_string();
    } else {
        // Default quantization for models without explicit quantization
        quantization_level = "Default".to_string();
    }
    
    (family, parameter_size, quantization_level, estimated_size)
}

#[derive(Debug, Serialize, Deserialize)]
pub struct OllamaInfo {
    pub installed: bool,
    pub version: Option<String>,
    pub path: Option<String>,
}

#[tauri::command]
async fn get_system_info() -> Result<SystemInfo, String> {
    let mut sys = System::new_all();
    sys.refresh_all();

    // Get platform and architecture
    let platform = std::env::consts::OS.to_string();
    let arch = std::env::consts::ARCH.to_string();

    // Get CPU info
    let cpu_info = CpuInfo {
        cores: num_cpus::get(),
        name: sys.cpus().first().map(|cpu| cpu.name().to_string()).unwrap_or_else(|| "Unknown".to_string()),
        memory: sys.total_memory(),
    };

    // Get GPU info
    let gpu_info = get_gpu_info().await;

    // Get Ollama info
    let ollama_info = get_ollama_info().await;

    Ok(SystemInfo {
        platform,
        arch,
        gpu: gpu_info,
        cpu: cpu_info,
        ollama: ollama_info,
    })
}

async fn get_gpu_info() -> GpuInfo {
    // Try to detect GPU using system commands
    let gpu_name = detect_gpu_name().await;
    
    GpuInfo {
        available: gpu_name.is_some(),
        name: gpu_name,
        memory: None,
        driver: None,
    }
}

async fn detect_gpu_name() -> Option<String> {
    // Try different methods to detect GPU
    if let Ok(output) = Command::new("nvidia-smi").arg("--query-gpu=name").arg("--format=csv,noheader,nounits").output() {
        if output.status.success() {
            if let Ok(name) = String::from_utf8(output.stdout) {
                return Some(name.trim().to_string());
            }
        }
    }

    if let Ok(output) = Command::new("lspci").arg("-nn").output() {
        if output.status.success() {
            if let Ok(output_str) = String::from_utf8(output.stdout) {
                for line in output_str.lines() {
                    if line.contains("VGA") || line.contains("3D") {
                        if line.contains("NVIDIA") {
                            return Some("NVIDIA GPU".to_string());
                        } else if line.contains("AMD") {
                            return Some("AMD GPU".to_string());
                        } else if line.contains("Intel") {
                            return Some("Intel GPU".to_string());
                        }
                    }
                }
            }
        }
    }

    None
}

async fn get_ollama_info() -> OllamaInfo {
    // Check if ollama is installed
    match which("ollama") {
        Ok(path) => {
            let path_str = path.to_string_lossy().to_string();
            
            // Try to get version
            let version = match Command::new("ollama").arg("--version").output() {
                Ok(output) if output.status.success() => {
                    String::from_utf8(output.stdout).ok()
                        .and_then(|v| v.lines().next().map(|line| line.trim().to_string()))
                }
                _ => None,
            };

            OllamaInfo {
                installed: true,
                version,
                path: Some(path_str),
            }
        }
        Err(_) => OllamaInfo {
            installed: false,
            version: None,
            path: None,
        },
    }
}

#[tauri::command]
async fn check_ollama_status() -> Result<bool, String> {
    match Command::new("ollama").arg("list").output() {
        Ok(output) => Ok(output.status.success()),
        Err(_) => Ok(false),
    }
}

#[tauri::command]
async fn get_ollama_models() -> Result<Vec<String>, String> {
    match Command::new("ollama").arg("list").output() {
        Ok(output) if output.status.success() => {
            let output_str = String::from_utf8(output.stdout)
                .map_err(|e| format!("Failed to parse ollama output: {}", e))?;
            
            let models: Vec<String> = output_str
                .lines()
                .skip(1) // Skip header
                .filter_map(|line| {
                    let parts: Vec<&str> = line.split_whitespace().collect();
                    parts.first().map(|s| s.to_string())
                })
                .collect();
            
            Ok(models)
        }
        Ok(_) => Ok(vec![]),
        Err(e) => Err(format!("Failed to run ollama list: {}", e)),
    }
}

#[tauri::command]
async fn pull_ollama_model(model_name: String) -> Result<String, String> {
    match Command::new("ollama").arg("pull").arg(&model_name).output() {
        Ok(output) if output.status.success() => {
            Ok(format!("Successfully pulled model: {}", model_name))
        }
        Ok(output) => {
            let error = String::from_utf8_lossy(&output.stderr);
            Err(format!("Failed to pull model {}: {}", model_name, error))
        }
        Err(e) => Err(format!("Failed to run ollama pull: {}", e)),
    }
}

#[tauri::command]
async fn chat_with_ollama(request: ChatRequest) -> Result<ChatResponse, String> {
    println!("Chat request received for model: {}", request.model);
    
    // First check if the model exists
    let list_output = Command::new("ollama").arg("list").output();
    match list_output {
        Ok(output) if output.status.success() => {
            let list_text = String::from_utf8_lossy(&output.stdout);
            if !list_text.contains(&request.model) {
                return Err(format!("Model '{}' is not installed. Available models: {}", 
                    request.model, 
                    list_text.lines().skip(1).collect::<Vec<_>>().join(", ")
                ));
            }
        }
        Ok(output) => {
            let error = String::from_utf8_lossy(&output.stderr);
            return Err(format!("Failed to list models: {}", error));
        }
        Err(e) => {
            return Err(format!("Failed to run 'ollama list': {}", e));
        }
    }
    
    let mut ollama_cmd = Command::new("ollama");
    ollama_cmd.arg("run").arg(&request.model);
    
    // Convert messages to a single prompt
    let prompt = format_messages_to_prompt(&request.messages);
    println!("Formatted prompt: {}", prompt);
    ollama_cmd.arg(&prompt);
    
    // Add options if provided
    if let Some(temp) = request.temperature {
        ollama_cmd.arg("--temperature").arg(temp.to_string());
    }
    
    println!("Running ollama command...");
    match ollama_cmd.output() {
        Ok(output) if output.status.success() => {
            let response_text = String::from_utf8_lossy(&output.stdout).to_string();
            println!("Raw ollama output: {}", response_text);
            
            // Parse the response to extract the actual content
            let content = extract_response_content(&response_text);
            println!("Extracted content: {}", content);
            
            if content.is_empty() {
                return Err("Received empty response from Ollama".to_string());
            }
            
            Ok(ChatResponse {
                content: content.clone(),
                usage: Some(TokenUsage {
                    prompt_tokens: prompt.len() as u32,
                    completion_tokens: content.len() as u32,
                    total_tokens: (prompt.len() + content.len()) as u32,
                }),
            })
        }
        Ok(output) => {
            let stdout = String::from_utf8_lossy(&output.stdout);
            let stderr = String::from_utf8_lossy(&output.stderr);
            Err(format!("Ollama command failed. Stdout: {}, Stderr: {}", stdout, stderr))
        }
        Err(e) => Err(format!("Failed to run ollama command: {}", e)),
    }
}

fn format_messages_to_prompt(messages: &[ChatMessage]) -> String {
    messages
        .iter()
        .map(|msg| {
            match msg.role.as_str() {
                "user" => format!("Human: {}\n", msg.content),
                "assistant" => format!("Assistant: {}\n", msg.content),
                "system" => format!("System: {}\n", msg.content),
                _ => format!("{}: {}\n", msg.role, msg.content),
            }
        })
        .collect::<String>()
        + "Assistant:"
}

fn extract_response_content(response: &str) -> String {
    // Remove ANSI escape sequences and terminal control characters
    let cleaned = response
        .replace("\x1b[?25h", "")
        .replace("\x1b[?25l", "")
        .replace("\x1b[?2026h", "")
        .replace("\x1b[?2026l", "")
        .replace("\x1b[1G", "")
        .replace("\x1b[2K", "")
        .replace("\x1b[K", "")
        .replace("\x1b[", "")
        .replace("\x1b", "");
    
    // Remove common Ollama output formatting
    let lines: Vec<&str> = cleaned.lines().collect();
    let mut content = String::new();
    
    for line in lines {
        // Skip empty lines and common Ollama output patterns
        if line.trim().is_empty() || 
           line.contains(">>>") || 
           line.starts_with(">>>") ||
           line.contains("⠙") || // Loading spinner characters
           line.contains("⠹") ||
           line.contains("⠸") ||
           line.contains("⠼") ||
           line.contains("⠴") ||
           line.contains("⠦") ||
           line.contains("⠧") ||
           line.contains("⠇") ||
           line.contains("⠏") ||
           line.contains("⠋") {
            continue;
        }
        
        // Remove the model name prefix if present
        let clean_line = if line.contains(":") {
            line.splitn(2, ':').nth(1).unwrap_or(line).trim()
        } else {
            line.trim()
        };
        
        if !clean_line.is_empty() {
            content.push_str(clean_line);
            content.push('\n');
        }
    }
    
    content.trim().to_string()
}

#[tauri::command]
async fn remove_ollama_model(model_name: String) -> Result<String, String> {
    match Command::new("ollama").arg("rm").arg(&model_name).output() {
        Ok(output) if output.status.success() => {
            Ok(format!("Successfully removed model: {}", model_name))
        }
        Ok(output) => {
            let error = String::from_utf8_lossy(&output.stderr);
            Err(format!("Failed to remove model {}: {}", model_name, error))
        }
        Err(e) => Err(format!("Failed to run ollama rm: {}", e)),
    }
}

#[tauri::command]
async fn install_model_from_url(model_name: String) -> Result<String, String> {
    println!("Installing model from URL: {}", model_name);
    
    // Check if model is already installed
    let list_output = Command::new("ollama").arg("list").output();
    match list_output {
        Ok(output) if output.status.success() => {
            let list_text = String::from_utf8_lossy(&output.stdout);
            if list_text.contains(&model_name) {
                return Ok(format!("Model '{}' is already installed", model_name));
            }
        }
        _ => {}
    }
    
    // Install the model
    match Command::new("ollama").arg("pull").arg(&model_name).output() {
        Ok(output) if output.status.success() => {
            Ok(format!("Successfully installed model: {}", model_name))
        }
        Ok(output) => {
            let error = String::from_utf8_lossy(&output.stderr);
            Err(format!("Failed to install model {}: {}", model_name, error))
        }
        Err(e) => Err(format!("Failed to run ollama pull: {}", e)),
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            get_system_info,
            check_ollama_status,
            get_ollama_models,
            get_all_ollama_models,
            pull_ollama_model,
            remove_ollama_model,
            chat_with_ollama,
            install_model_from_url
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
