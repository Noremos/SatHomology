import win32com
import win32com.client
import os.path

Application = win32com.client.Dispatch("PowerPoint.Application")
Presentation = Application.Presentations.Open(r"D:/JSON.pptx")
dpi = 300

# Export each slide as a PNG image with the specified DPI value
for i in range(Presentation.Slides.Count):
    slide = Presentation.Slides[i]
    slide.Export(os.path.abspath(f"./s/slide_{i}.png"), "PNG", 1920, 1080)

Application.Quit()
Presentation =  None
Application = None
