test_that("It returns NULL if the passed object is NULL", {
  expect_equal(castToRObject(NULL), NULL)
})

test_that("It returns base R types as is", {
  expect_equal(castToRObject(1), 1)
  expect_equal(castToRObject("a"), "a")
  expect_equal(castToRObject(TRUE), TRUE)
  expect_equal(castToRObject(FALSE), FALSE)
  expect_equal(castToRObject(1L), 1L)
  expect_equal(castToRObject(1.1), 1.1)
  expect_equal(castToRObject(1.1 + 1i), 1.1 + 1i)
  expect_equal(castToRObject(1:3), 1:3)
  expect_equal(castToRObject(c(1, 2, 3)), c(1, 2, 3))
  expect_equal(castToRObject(list(1, 2, 3)), list(1, 2, 3))
  expect_equal(
    castToRObject(data.frame(a = 1:3, b = c("a", "b", "c"))),
    data.frame(a = 1:3, b = c("a", "b", "c"))
  )
})

test_that("It returns a `NetObject` when a valid pointer is provided", {
  skip_if_no_dotnet()
  testClassName <- "ClrFacade.Tests.RefClasses.LevelOneClass"
  o <- .External(
    "r_create_clr_object",
    testClassName,
    PACKAGE = rSharpEnv$nativePkgName
  )
  netObj <- castToRObject(o)
  # Check if the object is a NetObject
  expect_true(inherits(netObj, "NetObject"))
})

test_that("It returns a list of `NetObject` when a list with valid pointers is provided", {
  skip_if_no_dotnet()
  testClassName <- "ClrFacade.Tests.RefClasses.LevelOneClass"
  o <- .External(
    "r_create_clr_object",
    testClassName,
    PACKAGE = rSharpEnv$nativePkgName
  )
  o2 <- .External(
    "r_create_clr_object",
    testClassName,
    PACKAGE = rSharpEnv$nativePkgName
  )
  netObj <- castToRObject(list(o, o2))
  # Check if the object is a NetObject
  expect_true(inherits(netObj[[1]], "NetObject"))
})

test_that("It returns a list of `NetObject` and integer when a list is provided", {
  skip_if_no_dotnet()
  testClassName <- "ClrFacade.Tests.RefClasses.LevelOneClass"
  o <- .External(
    "r_create_clr_object",
    testClassName,
    PACKAGE = rSharpEnv$nativePkgName
  )
  netObj <- castToRObject(list(o, 2))
  # Check if the object is a NetObject
  expect_true(inherits(netObj[[1]], "NetObject"))
  expect_equal(netObj[[2]], 2)
})
