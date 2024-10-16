using System;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MusicPlayer
{
    public partial class Form1 : Form
    {
        // P/Invoke declarations
        [DllImport("spdll.dll", CharSet = CharSet.Ansi)]
        private static extern void InitializeClient(IntPtr hwnd);

        [DllImport("spdll.dll", CharSet = CharSet.Ansi)]
        private static extern void ConnectToServer(string ipAddress, int port);

        [DllImport("spdll.dll", CharSet = CharSet.Unicode)]
        private static extern IntPtr GetFileList(out int count);

        [DllImport("spdll.dll", CharSet = CharSet.Unicode)]
        private static extern void RequestAudioTrack(string trackName);

        [DllImport("spdll.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void PauseStreaming(bool pause);

        [DllImport("spdll.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetVolume(float volume);

        [DllImport("spdll.dll")]
        private static extern void StreamAudio();

        [DllImport("spdll.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void StartStreaming();

        private System.Windows.Forms.Timer volumeUpdateTimer;
        private float currentVolume = 0.5f;
        private bool isPaused = false;
        private bool isPlaying = false;

        public Form1()
        {
            InitializeComponent();
            InitializeClient(this.Handle);

            // Initialize the volume update timer
            volumeUpdateTimer = new System.Windows.Forms.Timer
            {
                Interval = 100 // Update every 100 milliseconds
            };
            volumeUpdateTimer.Tick += VolumeUpdateTimer_Tick;
            volumeUpdateTimer.Start();

            // Initialize volume slider
            trackBar1.Value = (int)(currentVolume * trackBar1.Maximum);
            trackBar1.Scroll += volumeSlider_Scroll;

            // Bind Pause button click event
            Pause.Click += Pause_Click;
        }

        private void VolumeUpdateTimer_Tick(object sender, EventArgs e)
        {
            // Call SetVolume function from the DLL to update the volume
            SetVolume(currentVolume);
        }

        // Add the received available audio files
        private void PopulateFileList()
        {
            fileListBox.Items.Clear();
            IntPtr fileListPtr = GetFileList(out int count);
            string fileList = Marshal.PtrToStringUni(fileListPtr);
            string[] files = fileList.Split(new[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
            fileListBox.Items.AddRange(files);
        }

        // Connects to the server and populates the list
        private void Connect_Click(object sender, EventArgs e)
        {
            ConnectToServer("127.0.0.1", 54000);
            PopulateFileList();
        }


        // Plays the selected audio file
        private void Play_Click(object sender, EventArgs e)
        {
            if (fileListBox.SelectedItem != null)
            {
                if (!isPlaying)
                {
                    // Start new playback
                    string selectedFile = fileListBox.SelectedItem.ToString();
                    Task.Run(() => {
                        RequestAudioTrack(selectedFile);
                        StartStreaming();
                    });
                    isPlaying = true;
                    isPaused = false;
                }
                else if (isPaused)
                {
                    // Resume playback
                    PauseStreaming(false);
                    isPaused = false;
                }
            }
        }

        // Pause the audio playback
        private void Pause_Click(object sender, EventArgs e)
        {
            if (isPlaying)
            {
                    isPaused = !isPaused;
                PauseStreaming(isPaused);
            }
        }

        // Changes the volume of audio playback
            private void volumeSlider_Scroll(object sender, EventArgs e)
        {
            currentVolume = trackBar1.Value / (float)trackBar1.Maximum;

            SetVolume(currentVolume);
        }
    }
}
