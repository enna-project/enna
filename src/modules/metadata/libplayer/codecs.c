#include <string.h>

#include "codecs.h"

static const struct {
    const char *codec_id;
    const char *codec_name;
} codecs_mapping[] = {

    /* Audio Codecs */
    { "wma9dmo", "WMA 9" },
    { "wmadmo", "WMA" },
    { "wma9spdmo", "WMA 9 Speech" },
    { "wma9spdshow", "WMA 9 Speech" },
    { "ffqdm2", "QDM2" },
    { "qdmc", "QuickTime QDMC/QDM2" },
    { "qclp", "QuickTime QCLP" },
    { "qtmace3", "QuickTime MACE3" },
    { "qtmace6", "QuickTime MACE6" },
    { "ffra144", "RealAudio 1.0" },
    { "ffra288", "RealAudio 2.0" },
    { "ffcook", "COOK" },
    { "ffatrc", "Atrac 3" },
    { "ra144", "RealAudio 1.0" },
    { "ra144win", "RealAudio 1.0" },
    { "ra144mac", "RealAudio 1.0" },
    { "ra288", "RealAudio 2.0" },
    { "ra288win", "RealAudio 2.0" },
    { "ra288mac", "RealAudio 2.0" },
    { "ra10cook", "RealPlayer 10 COOK" },
    { "racook", "RealAudio COOK" },
    { "ra10cookwin", "RealAudio 10 COOK" },
    { "racookwin", "RealAudio COOK" },
    { "racookmac", "RealAudio COOK" },
    { "rasipr", "RealAudio Sipro" },
    { "ra10sipr", "RealAudio 10 Sipro" },
    { "ra10siprwin", "RealAudio 10 Sipro" },
    { "rasiprwin", "RealAudio Sipro" },
    { "rasiprmac", "RealAudio Sipro" },
    { "raatrc", "RealAudio ATRAC3" },
    { "ra10atrc", "RealAudio 10 ATRAC3" },
    { "ra10atrcwin", "RealAudio 10 ATRAC3" },
    { "raatrcwin", "RealAudio ATRAC3" },
    { "raatrcmac", "RealAudio ATRAC3" },
    { "ffadpcmimaamv", "AMV IMA ADPCM" },
    { "ffadpcmimaqt", "QT IMA ADPCM" },
    { "ffadpcmimawav", "WAV IMA ADPCM" },
    { "imaadpcm", "IMA ADPCM" },
    { "ffadpcmms", "MS ADPCM" },
    { "msadpcm", "MS ADPCM" },
    { "ffadpcmimadk4", "DK4 IMA ADPCM" },
    { "dk4adpcm", "Duck DK4 ADPCM" },
    { "ffadpcmimadk3", "DK3 IMA ADPCM" },
    { "dk3adpcm", "Duck DK3 ADPCM" },
    { "ffroqaudio", "Id RoQ" },
    { "ffsmkaud", "Smacker" },
    { "ffdsicinaudio", "Delphine CIN" },
    { "ff4xmadmpcm", "4XM ADPCM" },
    { "ffadpcmimaws", "Westwood IMA ADPCM" },
    { "ffwssnd1", "Westwood SND1" },
    { "ffinterplaydpcm", "Interplay DPCM" },
    { "ffadpcmea", "EA ADPCM" },
    { "ffadpcmeamaxis", "EA MAXIS XA ADPCM" },
    { "ffadpcmxa", "XA ADPCM" },
    { "ffxandpcm", "XAN DPCM" },
    { "ffadpcmthp", "THP ADPCM" },
    { "libdv", "Raw DV" },
    { "ffdv", "DV" },
    { "faad", "AAC" },
    { "ffaac", "AAC" },
    { "ffflac", "FLAC" },
    { "ffalac", "ALAC" },
    { "fftta", "True Audio (TTA)" },
    { "ffwavpack", "WavPack" },
    { "ffshorten", "Shorten" },
    { "ffape", "Monkey's Audio" },
    { "ffmlp", "MLP" },
    { "ffnellymoser", "Nellymoser" },
    { "pcm", "Uncompressed PCM" },
    { "divx", "DivX audio (WMA)" },
    { "msadpcmacm", "MS ADPCM" },
    { "mp3", "MP3" },
    { "ffpcmdaud", "D-Cinema" },
    { "ffwmav1", "DivX audio v1" },
    { "ffwmav2", "DivX audio v2" },
    { "ffmac3", "Macintosh Audio Compression and Expansion 3:1" },
    { "ffmac6", "Macintosh Audio Compression and Expansion 6:1" },
    { "ffsonic", "Sonic" },
    { "ffmp3on4", "Multi-channel MP3 on MP4" },
    { "ffmp3", "MP3" },
    { "ffmp3adu", "MP3 adu" },
    { "ffmp2", "MP2" },
    { "mad", "MP3" },
    { "mp3acm", "MP3" },
    { "imaadpcmacm", "IMA ADPCM" },
    { "msgsm", "MS GSM" },
    { "msgsmacm", "MS GSM" },
    { "msnaudio", "MSN AUDIO" },
    { "alaw", "aLaw" },
    { "ulaw", "uLaw" },
    { "dvdpcm", "Uncompressed DVD/VOB LPCM" },
    { "a52", "AC3" },
    { "ffac3", "AC-3" },
    { "ffeac3", "E-AC-3" },
    { "dts", "DTS" },
    { "ffdca", "DTS" },
    { "ffmusepack7", "Musepack sv7" },
    { "ffmusepack8", "Musepack sv8" },
    { "musepack", "Musepack" },
    { "ffamrnb", "AMR Narrowband" },
    { "ffamrwb", "AMR Wideband" },
    { "ffadcpmswf", "ADPCM Flash-variant" },
    { "voxware", "VoxWare" },
    { "acelp", "ACELP.net Sipro Lab" },
    { "ffimc", "Intel Music Coder" },
    { "imc", "Intel Music Coder" },
    { "iac25", "Indeo audio" },
    { "ffctadp32", "Creative ADPCM" },
    { "ctadp32", "Creative ADPCM" },
    { "sc4", "SC4 : Micronas speech" },
    { "hwac3", "AC-3" },
    { "hwdts", "DTS" },
    { "ffvorbis", "Vorbis" },
    { "vorbis", "OggVorbis" },
    { "vorbisacm", "OggVorbis ACM" },
    { "speex", "Speex" },
    { "vivoaudio", "Vivo G.723/Siren" },
    { "g72x", "G.711/G.721/G.723" },
    { "ffg726", "Sharp G.726" },
    { "g726", "Sharp G.726" },
    { "atrac3", "Sony ATRAC3" },
    { "ALF2", "ALF2" },
    { "fftruespeech", "TrueSpeech" },
    { "truespeech", "DSP Group TrueSpeech" },
    { "voxwarert24", "VoxWare RT24 speech" },
    { "lhacm", "Lernout & Hauspie CELP and SBC" },
    { "TwinVQ", "NTTLabs VQF" },
    { "hwmpa", "MPEG audio" },
    { "msnsiren", "MSN siren" },

    /* Video Codecs */
    { "ffmvi1", "Motion Pixels" },
    { "ffmdec", "Sony PlayStation MDEC" },
    { "ffsiff", "Beam Software SIFF" },
    { "ffmimic", "Mimic" },
    { "ffkmvc", "Karl Morton" },
    { "ffzmbv", "Zip Motion-Block Video" },
    { "zmbv", "Zip Motion-Block Video" },
    { "mpegpes", "MPEG-PES" },
    { "ffmpeg1", "MPEG-1" },
    { "ffmpeg2", "MPEG-2" },
    { "ffmpeg12", "MPEG-1/2" },
    { "mpeg12", "MPEG-1/2" },
    { "ffmpeg12mc", "MPEG-1/2" },
    { "ffnuv", "NuppelVideo" },
    { "nuv", "NuppelVideo" },
    { "ffbmp", "BMP" },
    { "ffgif", "GIF" },
    { "fftiff", "TIFF" },
    { "ffpcx", "PCX" },
    { "ffpng", "PNG" },
    { "mpng", "PNG" },
    { "ffptx", "V.Flash PTX" },
    { "fftga", "TGA" },
    { "mtga", "TGA" },
    { "sgi", "SGI" },
    { "ffsunras", "SUN Rasterfile" },
    { "ffindeo3", "Intel Indeo 3.1/3.2" },
    { "fffli", "Autodesk FLI/FLC Animation" },
    { "ffaasc", "Autodesk RLE" },
    { "ffloco", "LOCO" },
    { "ffqtrle", "QuickTime Animation (RLE)" },
    { "ffrpza", "QuickTime Apple Video" },
    { "ffsmc", "Apple Graphics (SMC)" },
    { "ff8bps", "Planar RGB (Photoshop)" },
    { "ffcyuv", "Creative YUV" },
    { "ffmsrle", "Microsoft RLE" },
    { "ffroqvideo", "Id RoQ" },
    { "lzo", "LZO compressed" },
    { "theora", "Theora" },
    { "msuscls", "MSU Screen Capture Lossless" },
    { "cram", "Microsoft Video 1" },
    { "ffcvid", "Cinepak Video" },
    { "cvidvfw", "Cinepak Video" },
    { "huffyuv", "HuffYUV" },
    { "ffvideo1", "Microsoft Video 1" },
    { "ffmszh", "AVImszh" },
    { "ffzlib", "AVIzlib" },
    { "cvidxa", "XAnim's Radius Cinepak" },
    { "ffhuffyuv", "HuffYUV" },
    { "ffv1", "FFV1" },
    { "ffsnow", "SNOW" },
    { "ffasv1", "ASUS V1" },
    { "ffasv2", "ASUS V2" },
    { "ffvcr1", "ATI VCR1" },
    { "ffcljr", "Cirrus Logic AccuPak" },
    { "ffsvq1", "Sorenson Video v1" },
    { "ff4xm", "4XM" },
    { "ffvixl", "Miro/Pinnacle VideoXL" },
    { "ffqtdrw", "QuickDraw" },
    { "ffindeo2", "Indeo 2" },
    { "ffflv", "Flash" },
    { "fffsv", "Flash Screen" },
    { "ffdivx", "DivX ;-) (MSMPEG-4 v3)" },
    { "ffmp42", "MSMPEG-4 v2" },
    { "ffmp41", "MSMPEG-4 v1" },
    { "ffwmv1", "WMV 1 / WMV 7" },
    { "ffwmv2", "WMV 2 / WMV 8" },
    { "ffwmv3", "WMV 3 / WMV 9" },
    { "ffvc1", "VC-1" },
    { "ffh264", "H.264" },
    { "ffsvq3", "Sorenson Video v3" },
    { "ffodivx", "MPEG-4" },
    { "ffwv1f", "WV1F MPEG-4" },
    { "fflibschroedinger", "Dirac" },
    { "fflibdirac", "Dirac" },
    { "xvid", "Xvid (MPEG-4)" },
    { "divx4vfw", "DivX4Windows-VFW" },
    { "divxds", "DivX ;-) (MSMPEG-4 v3)" },
    { "divx", "DivX ;-) (MSMPEG-4 v3)" },
    { "mpeg4ds", "Microsoft MPEG-4 v1/v2" },
    { "mpeg4", "Microsoft MPEG-4 v1/v2" },
    { "wmv9dmo", "WMV 9" },
    { "wmvdmo", "WMV" },
    { "wmv8", "WMV 8" },
    { "wmv7", "WMV 7" },
    { "wmvadmo", "WMV Adv" },
    { "wmvvc1dmo", "VC-1 Advanced Profile" },
    { "wmsdmod", "Windows Media Screen Codec 2" },
    { "ubmp4", "UB Video MPEG-4" },
    { "zrmjpeg", "Zoran MJPEG" },
    { "ffmjpeg", "MJPEG" },
    { "ffmjpegb", "MJPEG-B" },
    { "ijpg", "JPEG" },
    { "m3jpeg", "Morgan Motion JPEG" },
    { "mjpeg", "MainConcept Motion JPEG" },
    { "avid", "AVID Motion JPEG" },
    { "LEAD", "LEAD (M)JPEG" },
    { "imagepower", "ImagePower MJPEG2000" },
    { "m3jpeg2k", "Morgan MJPEG2000" },
    { "m3jpegds", "Morgan MJPEG" },
    { "pegasusm", "Pegasus Motion JPEG" },
    { "pegasusl", "Pegasus lossless JPEG" },
    { "pegasusmwv", "Pegasus Motion Wavelet 2000" },
    { "vivo", "Vivo H.263" },
    { "u263", "UB Video H.263/H.263+/H.263++" },
    { "i263", "I263" },
    { "ffi263", "I263" },
    { "ffh263", "H.263+" },
    { "ffzygo", "ZyGo" },
    { "h263xa", "XAnim's CCITT H.263" },
    { "ffh261", "CCITT H.261" },
    { "qt261", "QuickTime H.261" },
    { "h261xa", "XAnim's CCITT H.261" },
    { "m261", "M261" },
    { "indeo5ds", "Intel Indeo 5" },
    { "indeo5", "Intel Indeo 5" },
    { "indeo4", "Intel Indeo 4.1" },
    { "indeo3", "Intel Indeo 3.1/3.2" },
    { "indeo5xa", "XAnim's Intel Indeo 5" },
    { "indeo4xa", "XAnim's Intel Indeo 4.1" },
    { "indeo3xa", "XAnim's Intel Indeo 3.1/3.2" },
    { "qdv", "Sony Digital Video (DV)" },
    { "ffdv", "DV" },
    { "libdv", "Raw DV" },
    { "mcdv", "MainConcept DV" },
    { "3ivXxa", "XAnim's 3ivx Delta 3.5" },
    { "3ivX", "3ivx Delta 4.5" },
    { "rv3040", "RealPlayer 10 RV30/40" },
    { "rv3040win", "RealPlayer 10 RV30/40" },
    { "ffrv40", "RealPlayer 9 RV40" },
    { "rv40", "RealPlayer 9 RV40" },
    { "rv40win", "RealPlayer 9 RV40" },
    { "rv40mac", "RealPlayer 9 RV40" },
    { "rv30", "RealPlayer 8 RV30" },
    { "rv30win", "RealPlayer 8 RV30" },
    { "rv30mac", "RealPlayer 9 RV30" },
    { "ffrv20", "realPlayer 8 RV20" },
    { "rv20", "RealPlayer 8 RV20" },
    { "rv20winrp10", "RealPlayer 10 RV20" },
    { "rv20win", "RealPlayer 8 RV20" },
    { "rv20mac", "RealPlayer 9 RV20" },
    { "ffrv10", "RealPlayer 8 RV10" },
    { "alpary", "Alparysoft lossless codec" },
    { "alpary2", "Alparysoft lossless codec" },
    { "LEADMW20", "Lead CMW wavelet 2.0" },
    { "lagarith", "Lagarith Lossless Video" },
    { "psiv", "Infinite Video PSI_V" },
    { "canopushq", "Canopus HQ" },
    { "canopusll", "Canopus Lossless" },
    { "ffvp3", "On2 VP3" },
    { "fftheora", "Theora" },
    { "vp3", "On2 VP3" },
    { "vp4", "On2 VP4" },
    { "ffvp5", "On2 VP5" },
    { "vp5", "On2 VP5" },
    { "ffvp6", "On2 VP6" },
    { "ffvp6a", "On2 VP6A" },
    { "ffvp6f", "On2 VP6 Flash" },
    { "vp6", "On2 VP6" },
    { "vp7", "On2 VP7" },
    { "mwv1", "Motion Wavelets" },
    { "asv2", "ASUS V2" },
    { "asv1", "ASUS V1" },
    { "ffultimotion", "IBM Ultimotion" },
    { "ultimotion", "IBM Ultimotion" },
    { "mss1", "Windows Screen Video" },
    { "ucod", "UCOD-ClearVideo" },
    { "vcr2", "ATI VCR-2" },
    { "CJPG", "CJPG" },
    { "ffduck", "Duck Truemotion1" },
    { "fftm20", "Duck/On2 TrueMotion 2.0" },
    { "tm20", "TrueMotion 2.0" },
    { "ffamv", "Modified MJPEG" },
    { "ffsp5x", "SP5x" },
    { "sp5x", "SP5x" },
    { "vivd2", "SoftMedia ViVD V2" },
    { "winx", "Winnov Videum winx" },
    { "ffwnv1", "Winnov Videum wnv1" },
    { "wnv1", "Winnov Videum wnv1" },
    { "vdom", "VDOWave" },
    { "lsv", "Vianet Lsvx" },
    { "ffvmnc", "VMware video" },
    { "vmnc", "VMware video" },
    { "ffsmkvid", "Smacker" },
    { "ffcavs", "Chinese AVS" },
    { "ffdnxhd", "DNxHD" },
    { "qt3ivx", "3ivx" },
    { "qtactl", "QuickTime Streambox ACT-L2" },
    { "qtavui", "QuickTime Avid Meridien Uncompressed" },
    { "qth263", "QuickTime H.263" },
    { "qtrlerpza", "Quicktime RLE/RPZA" },
    { "qtvp3", "QuickTime VP3" },
    { "qtzygo", "Quicktime ZyGo" },
    { "qtbhiv", "QuickTime BeHereiVideo" },
    { "qtcvid", "QuickTime Cinepak" },
    { "qtindeo", "QuickTime Indeo" },
    { "qtmjpeg", "QuickTime MJPEG" },
    { "qtmpeg4", "QuickTime MPEG-4" },
    { "qtsvq3", "QuickTime SVQ3" },
    { "qtsvq1", "QuickTime SVQ1" },
    { "vsslight", "VSS Light" },
    { "vssh264", "VSS H.264" },
    { "vssh264old", "VSS H.264" },
    { "vsswlt", "VSS Wavelet" },
    { "zlib", "AVIzlib" },
    { "mszh", "AVImszh" },
    { "alaris", "Alaris VideoGramPiX" },
    { "vcr1", "ATI VCR-1" },
    { "pim1", "Pinnacle Hardware MPEG-1" },
    { "qpeg", "Q-Team's QPEG" },
    { "rricm", "rricm" },
    { "ffcamtasia", "TechSmith Camtasia" },
    { "camtasia", "TechSmith Camtasia" },
    { "ffcamstudio", "CamStudio" },
    { "fraps", "FRAPS" },
    { "fffraps", "FRAPS" },
    { "fftiertexseq", "Tiertex SEQ" },
    { "ffvmd", "Sierra VMD" },
    { "ffdxa", "Feeble Files DXA" },
    { "ffdsicinvideo", "Delphine CIN" },
    { "ffthp", "THP" },
    { "ffbfi", "BFI" },
    { "ffbethsoftvid", "Bethesda Software VID" },
    { "ffrl2", "RL2" },
    { "fftxd", "Renderware TeXture Dictionary" },
    { "xan", "XAN" },
    { "ffwc3", "XAN wc3" },
    { "ffidcin", "Id CIN" },
    { "ffinterplay", "Interplay Video" },
    { "ffvqa", "VQA" },
    { "ffc93", "C93" },
    { "rawrgb32", "RAW RGB32" },
    { "rawrgb24", "RAW RGB24" },
    { "rawrgb16", "RAW RGB16" },
    { "rawbgr32flip", "RAW BGR32" },
    { "rawbgr32", "RAW BGR32" },
    { "rawbgr24flip", "RAW BGR24" },
    { "rawbgr24", "RAW BGR24" },
    { "rawbgr16flip", "RAW BGR15" },
    { "rawbgr16", "RAW BGR15" },
    { "rawbgr15flip", "RAW BGR15" },
    { "rawbgr15", "RAW BGR15" },
    { "rawbgr8flip", "RAW BGR8" },
    { "rawbgr8", "RAW BGR8" },
    { "rawbgr1", "RAW BGR1" },
    { "rawyuy2", "RAW YUY2" },
    { "rawyuv2", "RAW YUV2" },
    { "rawuyvy", "RAW UYVY" },
    { "raw444P", "RAW 444P" },
    { "raw422P", "RAW 422P" },
    { "rawyv12", "RAW YV12" },
    { "rawnv21", "RAW NV21" },
    { "rawnv12", "RAW NV12" },
    { "rawhm12", "RAW HM12" },
    { "rawi420", "RAW I420" },
    { "rawyvu9", "RAW YVU9" },
    { "rawy800", "RAW Y8/Y800" },

    { NULL, NULL }
};

char *
get_codec_name (char *codec_id)
{
    int i;

    if (!codec_id)
        return strdup ("");
  
    for (i = 0; codecs_mapping[i].codec_id; i++)
        if (!strcmp (codec_id, codecs_mapping[i].codec_id))
            return strdup (codecs_mapping[i].codec_name);

    return strdup (codec_id);
}
