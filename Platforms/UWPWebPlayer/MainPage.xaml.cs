using EmbedIO;
using EmbedIO.Actions;
using EmbedIO.Files;
using Swan.Logging;
using System;
using System.IO;
using Windows.UI.Core;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace GamePlayerApp
{
    public sealed partial class MainPage : Page
    {

        static string url = "http://localhost:9696/";

        public MainPage()
        {

            Environment.SetEnvironmentVariable("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", "--disable-frame-rate-limit --disable-gpu-vsync --autoplay-policy=no-user-gesture-required");

            CreateWebServer(url).RunAsync();

            this.InitializeComponent();




            appwindow = Windows.UI.Core.CoreWindow.GetForCurrentThread();


            DispatcherTimer timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(16); // roughly 60 FPS
            timer.Tick += (sender, e) =>
            {
                // Update your image here.
                loop();
            };
            timer.Start();



        }

        Windows.UI.Core.CoreWindow appwindow;

        bool oldKeyDown = false;

        void loop()
        {


            bool keyDown = (appwindow.GetKeyState(Windows.System.VirtualKey.F11) & CoreVirtualKeyStates.Down) == CoreVirtualKeyStates.Down;


            if (keyDown != oldKeyDown && keyDown == true)
            {
                ToggleFullscreen();
            }

            oldKeyDown = keyDown;


        }

        private void GameWebView_FocusEngaged(Control sender, FocusEngagedEventArgs args)
        {
            GameWebView.Focus(FocusState.Unfocused);
            GameWebView.Visibility = Visibility.Collapsed;
            GameWebView.Visibility = Visibility.Visible;
        }

        private void CoreWindow_KeyDown1(CoreWindow sender, KeyEventArgs args)
        {
            if (args.VirtualKey == Windows.System.VirtualKey.F11 &&
                !args.KeyStatus.IsMenuKeyDown)
            {
                ToggleFullscreen();
                args.Handled = true;
            }
        }

        private void CoreWindow_KeyDown(CoreWindow sender, KeyEventArgs args)
        {
            if (args.VirtualKey == Windows.System.VirtualKey.F10 &&
                !args.KeyStatus.IsMenuKeyDown)
            {
                ToggleFullscreen();
                args.Handled = true;
            }
        }

        private void ToggleFullscreen()
        {
            var view = ApplicationView.GetForCurrentView();

            if (view.IsFullScreenMode)
            {
                view.ExitFullScreenMode();
            }
            else
            {
                view.TryEnterFullScreenMode();
            }

            // Force WebView to update its layout
            GameWebView.UpdateLayout();
        }

        private async void InitializeWebView2()
        {
            try
            {
                // Ensure CoreWebView2 is created before navigation
                await GameWebView.EnsureCoreWebView2Async();

            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"WebView2 Error: {ex.Message}");
            }
        }

        private static WebServer CreateWebServer(string url)
        {

            string filesRoot = HtmlRoot();

            var server = new WebServer(o => o
                    .WithUrlPrefix(url)
                    .WithMode(HttpListenerMode.EmbedIO))
                // First, we will configure our web server by adding Modules.
                .WithLocalSessionManager()
                .WithStaticFolder("/", filesRoot, true, m => m
                    .WithContentCaching(true)) // Add static files after other modules to avoid conflicts
                .WithModule(new ActionModule("/", HttpVerbs.Any, ctx => ctx.SendDataAsync(new { Message = "Error" })));

            server.AddCustomMimeType(".wasm", "application/wasm");
            server.AddCustomMimeType(".data", "application/octet-stream");
            server.AddCustomMimeType(".bin", "application/octet-stream");
            server.AddCustomMimeType(".symbols.json", "application/json");

            // Listen for state changes.
            server.StateChanged += (s, e) => $"WebServer New State - {e.NewState}".Info();

            return server;
        }



        private static string HtmlRoot()
        {
            // This will get the current WORKING directory (i.e. \bin\Debug)
            string workingDirectory = Environment.CurrentDirectory;
            // or: Directory.GetCurrentDirectory() gives the same result

            // This will get the current PROJECT directory
            string projectDirectory = Directory.GetCurrentDirectory();

            return Path.Combine(projectDirectory, "game");
        }
    }
}
