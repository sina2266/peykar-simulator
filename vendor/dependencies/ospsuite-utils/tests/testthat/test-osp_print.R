test_that("ospPrintClass prints class name correctly", {
  # Test with a data frame
  expect_snapshot(ospPrintClass(data.frame(x = 1:3, y = letters[1:3])))

  # Test with a list
  expect_snapshot(ospPrintClass(list(a = 1, b = 2)))

  # Test with a vector
  expect_snapshot(ospPrintClass(1:5))

  # Test with a custom R6 class
  myClass <- R6::R6Class("myClass")

  expect_snapshot(ospPrintClass(myClass$new()))
})

test_that("ospPrintHeader prints headers at different levels", {
  # Test with level 1 |>
  expect_snapshot(ospPrintHeader("Main Title", 1))

  # Test with level 2
  expect_snapshot(ospPrintHeader("Section Title", 2))

  # Test with level 3
  expect_snapshot(ospPrintHeader("Subsection Title", 3))

  # Test with invalid level
  expect_error(ospPrintHeader("Invalid", 4))
  expect_error(ospPrintHeader("Invalid", "not a number"))
})

test_that("ospPrintItems prints items correctly", {
  # Test with named vector
  person <- c(name = "John", age = "30", job = "Developer")
  expect_snapshot(ospPrintItems(person))

  # Test with unnamed vector
  colors <- c("red", "green", "blue")
  expect_snapshot(ospPrintItems(colors))

  # Test with named list
  my_list <- list(id = 123, values = c(1, 2, 3), name = "Test")
  expect_snapshot(ospPrintItems(my_list))

  # Test with mixed named and unnamed items
  mixed <- c(a = "A", "B", c = "C")
  expect_snapshot(ospPrintItems(mixed))

  # Test with title parameter for unnamed vector
  letters_vec <- c("A", "B", "C")
  expect_snapshot(ospPrintItems(letters_vec, title = "Letters"))

  # Test with title parameter for named vector
  numbered_letters <- c(A = 1, B = 2, C = 3)
  expect_snapshot(ospPrintItems(numbered_letters, title = "Letters"))
})

test_that("ospPrintItems handles empty values correctly", {
  # Test empty list
  empty_list <- list()
  expect_snapshot(ospPrintItems(
    empty_list,
    title = "Empty List",
    print_empty = TRUE
  ))
  expect_snapshot(ospPrintItems(
    empty_list,
    title = "Empty List",
    print_empty = FALSE
  ))

  # Test empty vector
  empty_vector <- numeric(0)
  expect_snapshot(ospPrintItems(
    empty_vector,
    title = "Empty Vector",
    print_empty = TRUE
  ))
  expect_snapshot(ospPrintItems(
    empty_vector,
    title = "Empty Vector",
    print_empty = FALSE
  ))

  # Test list with NULL values and empty vectors/lists
  list_with_nulls <- list(
    "Min" = NULL,
    "Max" = 100,
    "Unit" = NA,
    "EmptyVec" = character(0),
    "EmptyList" = list()
  )
  expect_snapshot(ospPrintItems(
    list_with_nulls,
    title = "Parameters",
    print_empty = TRUE
  ))
  expect_snapshot(ospPrintItems(
    list_with_nulls,
    title = "Parameters",
    print_empty = FALSE
  ))

  # Test list with all empty values
  all_empty_list <- list(
    "A" = NULL,
    "B" = NA,
    "C" = "",
    "D" = list(),
    "E" = numeric(0)
  )
  expect_snapshot(ospPrintItems(
    all_empty_list,
    title = "All Empty",
    print_empty = TRUE
  ))
  expect_snapshot(ospPrintItems(
    all_empty_list,
    title = "All Empty",
    print_empty = FALSE
  ))

  # Test unnamed list with mixed values
  unnamed_mixed <- list(NULL, 1, NA, "", list(), numeric(0))
  expect_snapshot(ospPrintItems(
    unnamed_mixed,
    title = "Unnamed Mixed",
    print_empty = TRUE
  ))
  expect_snapshot(ospPrintItems(
    unnamed_mixed,
    title = "Unnamed Mixed",
    print_empty = FALSE
  ))
})

test_that("Different OspPrint* functions work well together", {
  myClass <-
    R6::R6Class(
      "myClass",
      public = list(
        named_items = list(
          "A" = 1,
          "B" = 2,
          "C" = 3
        ),
        nested_item_list = list(
          sub_list_1 = list(
            "D" = 4,
            "E" = 5
          ),
          sub_list_2 = list(
            "F" = 6,
            "G" = 7
          )
        )
      )
    )
  my_object <- myClass$new()
  expect_snapshot({
    ospPrintClass(my_object)
    ospPrintItems(my_object$named_items, title = "Named Items")
    ospPrintHeader("Nested Item List", 2)
    purrr::iwalk(
      my_object$nested_item_list,
      \(x, idx) ospPrintItems(x, title = idx)
    )
  })
})
