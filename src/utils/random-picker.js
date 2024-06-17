// Create a function to randomly pick inside an array without repetiions
export default function (array, random = Math.random) {
  let pool = array.slice(0)
  return {
    next: function () {
      // Recreate the pool once consumed
      if (pool.length < 1) pool = array.slice(0)

      // Randomly pick the next item
      const index = Math.floor(random() * pool.length)
      const item = pool[index]

      // Remove the picked item from the pool
      pool.splice(index, 1)
      return item
    }
  }
}
