# ospPrintClass prints class name correctly

    Code
      ospPrintClass(data.frame(x = 1:3, y = letters[1:3]))
    Output
      <data.frame>

---

    Code
      ospPrintClass(list(a = 1, b = 2))
    Output
      <list>

---

    Code
      ospPrintClass(1:5)
    Output
      <integer>

---

    Code
      ospPrintClass(myClass$new())
    Output
      <myClass>

# ospPrintHeader prints headers at different levels

    Code
      ospPrintHeader("Main Title", 1)
    Output
      
      -- Main Title ------------------------------------------------------------------

---

    Code
      ospPrintHeader("Section Title", 2)
    Output
      
      -- Section Title --
      

---

    Code
      ospPrintHeader("Subsection Title", 3)
    Output
      
      -- Subsection Title 

# ospPrintItems prints items correctly

    Code
      ospPrintItems(person)
    Output
        * name: John
        * age: 30
        * job: Developer

---

    Code
      ospPrintItems(colors)
    Output
        * red
        * green
        * blue

---

    Code
      ospPrintItems(my_list)
    Output
        * id: 123
        * values: 1, 2, 3
        * name: Test

---

    Code
      ospPrintItems(mixed)
    Output
        * a: A
        * B
        * c: C

---

    Code
      ospPrintItems(letters_vec, title = "Letters")
    Output
      Letters:
        * A
        * B
        * C

---

    Code
      ospPrintItems(numbered_letters, title = "Letters")
    Output
      Letters:
        * A: 1
        * B: 2
        * C: 3

# ospPrintItems handles empty values correctly

    Code
      ospPrintItems(empty_list, title = "Empty List", print_empty = TRUE)
    Output
      Empty List:

---

    Code
      ospPrintItems(empty_list, title = "Empty List", print_empty = FALSE)
    Output
      Empty List:

---

    Code
      ospPrintItems(empty_vector, title = "Empty Vector", print_empty = TRUE)
    Output
      Empty Vector:

---

    Code
      ospPrintItems(empty_vector, title = "Empty Vector", print_empty = FALSE)
    Output
      Empty Vector:

---

    Code
      ospPrintItems(list_with_nulls, title = "Parameters", print_empty = TRUE)
    Output
      Parameters:
        * Min: NULL
        * Max: 100
        * Unit: NA
        * EmptyVec: <empty vector>
        * EmptyList: <empty list>

---

    Code
      ospPrintItems(list_with_nulls, title = "Parameters", print_empty = FALSE)
    Output
      Parameters:
        * Max: 100

---

    Code
      ospPrintItems(all_empty_list, title = "All Empty", print_empty = TRUE)
    Output
      All Empty:
        * A: NULL
        * B: NA
        * C: <empty string>
        * D: <empty list>
        * E: <empty vector>

---

    Code
      ospPrintItems(all_empty_list, title = "All Empty", print_empty = FALSE)
    Output
      All Empty:
        * All items are NULL, NA, or empty

---

    Code
      ospPrintItems(unnamed_mixed, title = "Unnamed Mixed", print_empty = TRUE)
    Output
      Unnamed Mixed:
        * NULL
        * 1
        * NA
        * <empty string>
        * <empty list>
        * <empty vector>

---

    Code
      ospPrintItems(unnamed_mixed, title = "Unnamed Mixed", print_empty = FALSE)
    Output
      Unnamed Mixed:
        * 1

# Different OspPrint* functions work well together

    Code
      ospPrintClass(my_object)
    Output
      <myClass>
    Code
      ospPrintItems(my_object$named_items, title = "Named Items")
    Output
      Named Items:
        * A: 1
        * B: 2
        * C: 3
    Code
      ospPrintHeader("Nested Item List", 2)
    Output
      
      -- Nested Item List --
      
    Code
      purrr::iwalk(my_object$nested_item_list, function(x, idx) ospPrintItems(x,
        title = idx))
    Output
      sub_list_1:
        * D: 4
        * E: 5
      sub_list_2:
        * F: 6
        * G: 7

