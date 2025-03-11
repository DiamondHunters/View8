#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "libplatform/libplatform.h"
#include "v8.h"

#pragma comment(lib, "v8_libbase.lib")
#pragma comment(lib, "v8_libplatform.lib")
#pragma comment(lib, "wee8.lib")

#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dmoguids.lib")
#pragma comment(lib, "wmcodecdspuuid.lib")
#pragma comment(lib, "msdmo.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "DbgHelp.lib")

using namespace v8;

static Isolate* isolate = nullptr;

static void loadBytecode(uint8_t* bytecodeBuffer, int length) {
  HandleScope handle_scope(isolate);
  Local<Context> context = Context::New(isolate);
  Context::Scope context_scope(context);

  // Load code into code cache.
  ScriptCompiler::CachedData* cached_data =
      new ScriptCompiler::CachedData(bytecodeBuffer, length);

  // Create dummy source.
  Local<String> resource_name = 
      String::NewFromUtf8(isolate, "code.jsc").ToLocalChecked();
  ScriptOrigin origin(resource_name);  // 修改这里，移除 isolate 参数

  Local<String> source_string = 
      String::NewFromUtf8(isolate, "\"ಠ_ಠ\"").ToLocalChecked();
  
  ScriptCompiler::Source source(source_string, origin, cached_data);

  // Compile code from code cache to print disassembly.
  MaybeLocal<UnboundScript> script = ScriptCompiler::CompileUnboundScript(
      isolate, &source, ScriptCompiler::kConsumeCodeCache);

  if (!script.IsEmpty()) {
    Local<UnboundScript> local_script = script.ToLocalChecked();
    Local<Script> bound_script = local_script->BindToCurrentContext();
    bound_script->Run(context).ToLocalChecked();
  }
}

static void readAllBytes(const std::string& file, std::vector<char>& buffer) {
  std::ifstream infile(file, std::ios::binary);

  infile.seekg(0, infile.end);
  size_t length = infile.tellg();
  infile.seekg(0, infile.beg);

  if (length > 0) {
    buffer.resize(length);
    infile.read(&buffer[0], length);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Please provide a file path as argument" << std::endl;
    return 1;
  }

  V8::SetFlagsFromString("--no-lazy --no-flush-bytecode");

  V8::InitializeICU();
  std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
  V8::InitializePlatform(platform.get());
  V8::Initialize();

  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      ArrayBuffer::Allocator::NewDefaultAllocator();

  isolate = Isolate::New(create_params);
  Isolate::Scope isolate_scope(isolate);
  HandleScope scope(isolate);

  std::vector<char> data;
  readAllBytes(argv[1], data);
  loadBytecode((uint8_t*)data.data(), data.size());

  // Cleanup
  isolate->Dispose();
  V8::Dispose();
  V8::DisposePlatform();
  delete create_params.array_buffer_allocator;

  return 0;
}
