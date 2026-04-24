````md
# Python / Gemini Setup
# This was written partially with AI because I am too tired to write the readme... :(

The C++ game does not run inside the Python virtual environment. The game runs normally, but when it calls the AI bridge, it should call the Python executable inside `.venv`.

## 1. Create the virtual environment

From the project root:

```cmd
py -3 -m venv .venv
````

## 2. Activate it

```cmd
.venv\Scripts\activate
```

## 3. Install requirements

This will depend on your python installation so adjust accordingly

```cmd
py -3 -m pip install -r requirements.txt
```

Your `requirements.txt` should contain:

```txt
google-genai
```

## 4. Test Python Gemini import

```cmd
python -c "from google import genai; print('ok')"
```

If it prints `ok`, the Python setup works.

## 5. Set your Gemini API key

In the same terminal before running the game:

You can acquire this token for free by logging into google ai studio's api key page:

https://aistudio.google.com/api-keys?

Additionally, you can message the dev at rj2387@nyu.edu on email or slack 

```cmd
set GEMINI_API_KEY=your_key_here
```

Check it:

```cmd
echo %GEMINI_API_KEY%
```

## 6. C++ should call venv Python

Use this in the Windows branch of `callAI(...)`:

```cpp
std::string command = ".venv\\Scripts\\python.exe ai\\gemini_bridge.py --key %GEMINI_API_KEY% --type " + promptType + " --output data\\ai_output.txt";
```

## 7. Manual bridge test

```cmd
.venv\Scripts\python.exe ai\gemini_bridge.py --key "%GEMINI_API_KEY%" --type biome --output data\ai_output.txt --grid-x 0 --grid-y 0
```

Then check:

```txt
data/ai_output.txt
```

```
```
