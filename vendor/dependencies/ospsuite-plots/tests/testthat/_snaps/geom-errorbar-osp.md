# width must be a single non-negative finite number

    Code
      geom_errorbar_osp(width = -1)
    Condition
      Error in `geom_errorbar_osp()`:
      ! Assertion on 'width' failed: Element 1 is not >= 0.

---

    Code
      geom_errorbar_osp(width = c(1, 2))
    Condition
      Error in `geom_errorbar_osp()`:
      ! Assertion on 'width' failed: Must have length 1.

---

    Code
      geom_errorbar_osp(width = "wide")
    Condition
      Error in `geom_errorbar_osp()`:
      ! Assertion on 'width' failed: Must be of type 'number', not 'character'.

