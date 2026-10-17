# Checks if Printable prints properly

    Code
      x
    Condition
      Warning:
      ospsuite.utils::Printable was deprecated in ospsuite.utils 1.6.2.
      i Please use ospsuite.utils::ospPrint*() instead.
    Output
      myPrintable: 
    Condition
      Warning:
      ospsuite.utils::Printable was deprecated in ospsuite.utils 1.6.2.
      i Please use ospsuite.utils::ospPrint*() instead.
    Output
         x: NULL 
    Condition
      Warning:
      ospsuite.utils::Printable was deprecated in ospsuite.utils 1.6.2.
      i Please use ospsuite.utils::ospPrint*() instead.
    Output
         y: NULL 

# Checks if Printable subclass cloning works as expected

    Code
      newPrintable1$new()$clone()
    Condition
      Warning:
      ospsuite.utils::Printable was deprecated in ospsuite.utils 1.6.2.
      i Please use ospsuite.utils::ospPrint*() instead.
    Output
      <newPrintable1>
        Inherits from: <Printable>
        Public:
          clone: function (deep = FALSE) 
          initialize: function () 
        Private:
          deprecated: function () 
          printClass: function () 
          printLine: function (entry, value = NULL, addTab = TRUE) 

