# tcolor --
# This script implements a simple color editor, where you can
# create colors using either the RGB, HSB, or CYM color spaces
# and apply the color to existing applications.

# Global variables that control the program:
#
# colorSpace -              Color space currently being used for
#                           editing.  Must be "rgb", "cmy", or "hsb".
# label1, label2, label3 -  Labels for the scales.
# red, green, blue -        Current color intensities in decimal
#                           on a scale of 0-65535.
# color -                   A string giving the current color value
#                           in the proper form for x:
#                           #RRRRGGGGBBBB
# updating -                Non-zero means that we're in the middle of
#                           updating the scales to load a new color,so
#                           information shouldn't be propagating back
#                           from the scales to other elements of the
#                           program:  this would make an infinite loop.
# command -                 Holds the command that has been typed
#                           into the "Command" entry.
# autoUpdate -              1 means execute the update command
#                           automatically whenever the color changes.
# name -                    Name for new color, typed into entry.

BEGIN {
  create_interface()
  changeColorSpace("hsb")
  tk_mainloop()
  
  # Many of these functions are only called via Tk interface callbacks.
  # Because Awka doesn't know this, it will think no calls are made
  # to some functions, and hence it won't output code for them.  These
  # dummy calls below ensures that each function is produced in the
  # translated code.
  if (0)
  {
    doUpdate()
    hsbToRgb()
    rgbToHsv()
    tc_loadNamedColor()
    tc_setScales()
    tc_scaleChanged()
    error()
  }
}
  
function error(msg) {
  print msg >"/dev/stderr"
  exit
}

function create_interface() {
  tk("wm title . \"Color Editor\"")

  tk("set colorSpace hsb");      colorSpace="hsb"
  tk("set red 65535");           red=65535
  tk("set green 0");             green=0
  tk("set blue 0");              blue=0
  tk("set color #ffff00000000"); color="#ffff00000000"
  tk("set updating 0");          updating=0
  tk("set autoUpdate 1");        autoUpdate=1
  tk("set name \"\"");           name=""

  # Create the menu bar at the top of the window.

  tk("frame .menu -relief raised -borderwidth 2")
  tk("pack .menu -side top -fill x")
  tk("menubutton .menu.file -text File -menu .menu.file.m -underline 0")
  tk("menu .menu.file.m")
  tk(".menu.file.m add radio -label \"RGB color space\" -variable colorSpace -value rgb -underline 0 -command {changeColorSpace rgb}")
  tk(".menu.file.m add radio -label \"CMY color space\" -variable colorSpace -value cmy -underline 0 -command {changeColorSpace cmy}")
  tk(".menu.file.m add radio -label \"HSB color space\" -variable colorSpace -value hsb -underline 0 -command {changeColorSpace hsb}")
  tk(".menu.file.m add separator")
  tk(".menu.file.m add radio -label \"Automatic updates\" -variable autoUpdate -value 1 -underline 0")
  tk(".menu.file.m add radio -label \"Manual updates\" -variable autoUpdate -value 0 -underline 0")
  tk(".menu.file.m add separator")
  tk(".menu.file.m add command -label \"Exit program\" -underline 0 -command \"destroy .\"")
  tk("pack .menu.file -side left")

  # Create the command entry window at the bottom of the window, along
  # with the update button.

  tk("frame .bot -relief raised -borderwidth 2")
  tk("pack .bot -side bottom -fill x")
  tk("label .commandLabel -text \"Command:\"")
  tk("entry .command -relief sunken -borderwidth 2 -textvariable command -font {Courier 12}")
  tk("button .update -text Update -command doUpdate")
  tk("pack .commandLabel -in .bot -side left")
  tk("pack .update -in .bot -side right -pady .1c -padx .25c")
  tk("pack .command -in .bot -expand yes -fill x -ipadx 0.25c")

  # Create the listbox that holds all of the color names in rgb.txt,
  # if an rgb.txt file can be found.

  tk("frame .middle -relief raised -borderwidth 2")
  tk("pack .middle -side top -fill both")
  
  i = 1
  for (num=split("/usr/local/lib/X11/rgb.txt /usr/lib/X11/rgb.txt /X11/R5/lib/X11/rgb.txt /X11/R4/lib/rgb/rgb.txt /usr/openwin/lib/X11/rgb.txt",F); i<=num; i++)
  {
    first_time = 1
    while( getline<F[i] >0 )
    {
      if (first_time)
      {
        tk("frame .middle.left")
        tk("pack .middle.left -side left -padx .25c -pady .25c")
        tk("listbox .names -width 20 -height 12 -yscrollcommand \".scroll set\" -relief sunken -borderwidth 2 -exportselection false")
        tk("bind .names <Double-1> { tc_loadNamedColor [.names get [.names curselection]] }")
        tk("scrollbar .scroll -orient vertical -command \".names yview\" -relief sunken -borderwidth 2")
        tk("pack .names -in .middle.left -side left")
        tk("pack .scroll -in .middle.left -side right -fill y")
        first_time = 0
      }
      if (NF == 4)
        tk(".names insert end "$4)
    }
    if (!first_time)
    {
      close(F[i])
      break
    }
  }

  # Create the three scales for editing the color, and the entry for
  # typing in a color value.

  tk("frame .middle.middle")
  tk("pack .middle.middle -side left -expand yes -fill y")
  tk("frame .middle.middle.1")
  tk("frame .middle.middle.2")
  tk("frame .middle.middle.3")
  tk("frame .middle.middle.4")
  tk("pack .middle.middle.1 .middle.middle.2 .middle.middle.3 -side top -expand yes")
  tk("pack .middle.middle.4 -side top -expand yes -fill x")
  for (i=1; i<=3; i++)
  {
    tk("label .label"i" -textvariable label"i)
    tk("scale .scale"i" -from 0 -to 1000 -length 6c -orient horizontal -command tc_scaleChanged")
    tk("pack .scale"i" .label"i" -in .middle.middle."i" -side top -anchor w")
  }

  tk("label .nameLabel -text \"Name:\"")
  tk("entry .name -relief sunken -borderwidth 2 -textvariable name -width 10 -font {Courier 12}")
  tk("pack .nameLabel -in .middle.middle.4 -side left")
  tk("pack .name -in .middle.middle.4 -side right -expand 1 -fill x")
  tk("bind .name <Return> {tc_loadNamedColor $name}")

  # Create the color display swatch on the right side of the window.

  tk("frame .middle.right")
  tk("pack .middle.right -side left -pady .25c -padx .25c -anchor s")
  tk("frame .swatch -width 2c -height 5c -background $color")
  tk("label .value -textvariable color -width 13 -font {Courier 12}")
  tk("pack .swatch -in .middle.right -side top -expand yes -fill both")
  tk("pack .value -in .middle.right -side bottom -pady .25c")
}

# The procedure below is invoked when one of the scales is adjusted.
# It propagates color information from the current scale readings
# to everywhere else that it is used.

function tc_scaleChanged(args) {
    if (updating) return
    colorSpace = tk_getvar("colorSpace")
    if (colorSpace == "rgb") 
    {
      red   = tk(".scale1 get") * 65.535
      green = tk(".scale2 get") * 65.535
      blue  = tk(".scale3 get") * 65.535
    } 
    else 
    {
      if (colorSpace == "cmy")
      {
        red   = 65535 - tk(".scale1 get") * 65.535
        green = 65535 - tk(".scale2 get") * 65.535
        blue  = 65535 - tk(".scale3 get") * 65.535
      }
      else
      {
        list = hsbToRgb(tk(".scale1 get")/1000.0, tk(".scale2 get")/1000.0, tk(".scale3 get")/1000.0)
        split(list, F)
        red =   F[1]
        green = F[2]
        blue =  F[3]
      }
    }
    tk("set color [format \"#%04x%04x%04x\" "int(red+0.5)" "int(green+0.5)" "int(blue+0.5)"]")
    tk(".swatch config -bg $color")
    if (tk_getvar("autoUpdate")) doUpdate()
    tk("update idletasks")
}

# The procedure below is invoked to update the scales from the
# current red, green, and blue intensities.  It's invoked after
# a change in the color space and after a named color value has
# been loaded.

function tc_setScales() {
    updating = 1
    colorSpace = tk_getvar("colorSpace")
    if (colorSpace == "rgb") 
    {
      tk(".scale1 set [format %.0f "red/65.535"]")
      tk(".scale2 set [format %.0f "green/65.535"]")
      tk(".scale3 set [format %.0f "blue/65.535"]")
    } 
    else 
    {
      if (colorSpace == "cmy") 
      {
        tk(".scale1 set [format %.0f "(65535-red)/65.535"]")
        tk(".scale2 set [format %.0f "(65535-green)/65.535"]")
        tk(".scale3 set [format %.0f "(65535-blue)/65.535"]")
      } 
      else 
      {
        list = rgbToHsv(red, green, blue)
        split(list, F)
        tk(".scale1 set [format %.0f "F[1] * 1000.0"]")
        tk(".scale2 set [format %.0f "F[2] * 1000.0"]")
        tk(".scale3 set [format %.0f "F[3] * 1000.0"]")
      }
    }
    updating = 0
}

# The procedure below is invoked when a named color has been
# selected from the listbox or typed into the entry.  It loads
# the color into the editor.

function tc_loadNamedColor(name) {
    if (left(name, 1) != "#") 
    {
      list = tk("winfo rgb .swatch "name)
      split(list, F)
      red = F[1]
      green = F[2]
      blue = F[3]
    } 
    else 
    {
      len = length(name)
      if (len == 4)
      { shift=12; format="%1x"; slen=1 }
      else if (len == 7)
      { shift=8; format="%2x"; slen=2 }
      else if (len == 10)
      { shift=4; format="%3x"; slen=3 }
      else if (len == 13)
      { shift=0; format="%4x"; slen=4 }
      else
        error("syntax error in color name: "name)

      red =   sprintf(format,substr(name,2,slen)) + 0
      green = sprintf(format,substr(name,2+slen,slen)) + 0
      blue =  sprintf(format,substr(name,2+slen+slen,slen)) + 0

      if (shift)
      {
        red   = lshift(red, shift)
        green = lshift(green, shift)
        blue  = lshift(blue, shift)
      }
    }
    tc_setScales()
    tk_setvar("color", sprintf("#%04x%04x%04x",red,green,blue))
    tk(".swatch config -bg $color")
    if (tk_getvar("autoUpdate")) doUpdate()
}

# The procedure below is invoked when a new color space is selected.
# It changes the labels on the scales and re-loads the scales with
# the appropriate values for the current color in the new color space

function changeColorSpace(space) {
    if (space == "rgb") 
    {
        tk_setvar("label1","Red")
        tk_setvar("label2","Green")
        tk_setvar("label3","Blue")
        tc_setScales()
        return
    }
    if (space == "cmy") 
    {
        tk_setvar("label1","Cyan")
        tk_setvar("label2","Magenta")
        tk_setvar("label3","Yellow")
        tc_setScales()
        return
    }
    if (space == "hsb") 
    {
        tk_setvar("label1","Hue")
        tk_setvar("label2","Saturation")
        tk_setvar("label3","Brightness")
        tc_setScales()
        return
    }
}

# The procedure below converts an RGB value to HSB.  It takes red, green,
# and blue components (0-65535) as arguments, and returns a list containing
# HSB components (floating-point, 0-1) as result.  The code here is a copy
# of the code on page 615 of "Fundamentals of Interactive Computer Graphics"
# by Foley and Van Dam.

function rgbToHsv(red,green,blue) {
    if (red > green) 
    {
      max = red
      min = green
    } 
    else 
    {
      max = green
      min = red
    }
    if (blue > max) 
      max = blue
    else if (blue < min)
      min = blue

    range = max-min
    sat = (max == 0 ? 0 : (max-min)/max)

    if (sat == 0)
        hue = 0
     else 
     {
        rc = (max - red) / range
        gc = (max - green) / range
        bc = (max - blue) / range
        if (red == max)
          hue = .166667 * (bc - gc)
        else if (green == max) 
          hue = .166667 * (2 + rc - bc)
        else
          hue = .166667 * (4 + gc - rc)

        if (hue < 0.0)
          hue++
    }
    return hue" "sat" "max/65535
}

# The procedure below converts an HSB value to RGB.  It takes hue, saturation,
# and value components (floating-point, 0-1.0) as arguments, and returns a
# list containing RGB components (integers, 0-65535) as result.  The code
# here is a copy of the code on page 616 of "Fundamentals of Interactive
# Computer Graphics" by Foley and Van Dam.

function hsbToRgb(hue,sat,value) {
    convfmt = CONVFMT
    CONVFMT = "%0.f"
    v = (65535.0 * value) ""
    if (!sat)
    {
      CONVFMT = convfmt
      return v" "v" "v
    }
    else 
    {
      hue *= 6.0
      if (hue >= 6.0)
        hue = 0.0
      
      i = int(hue)
      f = hue - i
      p = (65535.0 * value * (1 - sat)) ""
      q = (65535.0 * value * (1 - (sat * f))) ""
      t = (65535.0 * value * (1 - (sat * (1 - f)))) ""

      CONVFMT = convfmt
      
      if (!i)
        return v" "t" "p
      if (i == 1)
        return q" "v" "p
      if (i == 2)
        return p" "v" "t
      if (i == 3)
        return p" "q" "v
      if (i == 4)
        return t" "p" "v
      if (i == 5)
        return v" "p" "q
      error("i value "i" is out of range")
    }
}

# The procedure below is invoked when the "Update" button is pressed,
# and whenever the color changes if update mode is enabled.  It
# propagates color information as determined by the command in the
# Command entry.

function doUpdate() {
    tk("set newCmd $command")
    tk("regsub -all %% $command $color newCmd")
    tk("eval $newCmd")
}

#function tk(str) {
#}

#function tk_setvar(s1, s2) {
#}

#function tk_getvar(s1) {
#}

#function tk_mainloop() {
#}

#function lshift(s1, s2) {
#}

#function left(s1, s2) {
#}


