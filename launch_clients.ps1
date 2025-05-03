param(
    [Parameter(Mandatory=$true)]
    [int]$NumClients
)

$ScriptRoot = $PSScriptRoot
$ClientExePath = Join-Path -Path $ScriptRoot -ChildPath "build\bin\Release\ClientStreaming.exe"
$ClientArgs = @(
    "--server-port", "9000",
    "--multicast-address", "239.255.0.1",
    "--log-level", "error",
    "--fps", "30",
    "--cell-size", "10"
)

if ($NumClients -le 0) {
    Write-Error "Invalid number of clients provided: $NumClients. Please provide a positive integer."
    exit 1
}

if (-not (Test-Path -Path $ClientExePath -PathType Leaf)) {
    Write-Error "Client executable not found at '$ClientExePath'. Please ensure you have built the project in Release mode (e.g., run the 'cmake build release' task)."
    exit 1
}

Write-Host "Launching $NumClients client(s)..."

for ($i = 1; $i -le $NumClients; $i++) {
    Write-Host "Starting client $i..."
    Start-Process -FilePath $ClientExePath -ArgumentList $ClientArgs -WindowStyle Normal 
}

Write-Host "Done."
