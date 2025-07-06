@echo off
REM Check if commit message argument is provided
if "%~1"=="" (
    echo Please provide a commit message.
    echo Usage: gitcommit "Your commit message"
    exit /b 1
)

REM Add all changes
git add .

REM Commit with the passed message
git commit -m "%~1"

REM Push to the current branch
git push

echo Committed and Pushed all files!
